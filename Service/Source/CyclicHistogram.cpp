#include "CyclicHistogram.h"
#include <chrono>

CyclicHistogram::CyclicHistogram(const string &analyzer_schema, uint32_t nominalVoltage, size_t voltageBinSize)
{
    this->nominalVoltage = nominalVoltage;
    this->voltageBinSize = voltageBinSize;
    // Setup database structure
    DB = new Database(analyzer_schema);
    DB->ExecResource("SQL/Database_CyclicHistogram.sql");
    DB->ExecSentenceNoResult("DELETE FROM cychist");
    LOGFile::Debug("Cyclic Histogram database tables created\n");

    skippedFirstPointSet = false;
    startTime = 0;
}

CyclicHistogram::~CyclicHistogram()
{
    // Cleanup memory
}

void CyclicHistogram::AddPoints(float *pVoltageSamples, size_t arrBlockSize, uint8_t numPhases, float sampleRate, float nominalFrequency)
{
    uint32_t numSamplesCycle = sampleRate / nominalFrequency;

    // Log the current time as the end time for this histogram
    endTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if (startTime == 0)
    {
        // Log the current time as the start time for this histogram
        startTime = endTime;
    }

    // Skip the first requested set of data points (tends to be unreliable data)
    if (!skippedFirstPointSet)
    {
        LOGFile::Warning("[CH] Skipping first set of data.\n");
        skippedFirstPointSet = true;
        return;
    }

    //if(phaseNumber == 0){
    //    LOGFile::Error((to_string(pVoltageSamples[0]) + " | " + to_string(pVoltageSamples[1]) + " | " + to_string(pVoltageSamples[2]) + "\n").c_str());
    //}

    // Check if vector is large enough for number of phases
    if (numPhases > cumulativeCount.size())
    {
        ResizePhaseNumber(numPhases);
    }

    // Check if inner vectors are large enough to hold individual phase's samples
    for (uint8_t phaseNumber = 0; phaseNumber < numPhases; phaseNumber++)
    {
        if (numSamplesCycle > cumulativeCount[phaseNumber].size())
        {
            // Resize the cumulative count array to fit all data
            ResizeCumulativeCount(phaseNumber, numSamplesCycle);
        }
    }

    bool foundFirstCycle = false;
    uint32_t cycleStartIdx = 0;
    float cycleXDiff = 0;
    // Find the index of each zero-crossing instance (from - to +)
    for (uint32_t i = 0; i < arrBlockSize; i++)
    {
        for (uint8_t phaseNumber = 0; phaseNumber < numPhases; phaseNumber++)
        {
            if (i < arrBlockSize - 1)
            {
                float voltage_current = pVoltageSamples[(arrBlockSize * phaseNumber) + i];
                float voltage_next = pVoltageSamples[(arrBlockSize * phaseNumber) + i + 1]; // Look-ahead to next voltage

                float m = voltage_next - voltage_current;
                float b = voltage_current - (m * i); // b = y - (m * x)

                // Attempt to detect zero-crossing location
                if ((phaseNumber == 0) && (voltage_current < 0) && (voltage_current >= -0.05 * nominalVoltage) && (voltage_next > 0) && (voltage_next <= 0.05 * nominalVoltage))
                {
                    foundFirstCycle = true;
                    cycleStartIdx = i;

                    // Find the x-diff variable (amount to shift the entire waveform by)
                    float x0 = (0 - b) / m; // x = -b / m (calculates x value where y-intercept is 0)
                    cycleXDiff = x0 - i;
                }

                if (foundFirstCycle)
                {
                    float voltageNew = (m * (i + cycleXDiff)) + b;

                    // Don't attempt to add to cumulative count if the voltage is clipped
                    if (abs(voltageNew) <= nominalVoltage)
                    {
                        // Convert voltage measurement into a discrete bin number (i.e. quantization)
                        uint32_t binIndex = ((voltageNew + nominalVoltage) / (2 * nominalVoltage)) * (voltageBinSize - 1);

                        size_t sampleIdx = i - cycleStartIdx;
                        if (sampleIdx < numSamplesCycle)
                        {
                            // Perform the incremental count update
                            cumulativeCount[phaseNumber][sampleIdx][binIndex] += 1;
                        }
                    }
                }
            }
        }
    }
}

void CyclicHistogram::FlushToDisk()
{
    std::ofstream outputFile;
    outputFile.open("cyclichistogram.txt", std::ios::out);
    outputFile << "phase,sample,voltage_bin,count" << endl;

    for (size_t i = 0; i < cumulativeCount.size(); i++)
    {
        for (size_t j = 0; j < cumulativeCount[i].size(); j++)
        {
            for (size_t k = 0; k < cumulativeCount[i][j].size(); k++)
            {
                uint64_t value = cumulativeCount[i][j][k];
                if (value != 0)
                {
                    outputFile << i << "," << j << "," << k << "," << value << endl;
                }
            }
        }
    }

    outputFile.close();
}

void CyclicHistogram::FlushToDatabase(uint64_t event_starttime, string event_type)
{
    DB->ExecSentenceNoResult("BEGIN TRANSACTION;");

    string sentence = "INSERT INTO cychist (starttime, endtime, event_starttime, event_type) VALUES (" + DB->Bind(startTime) + ", " + DB->Bind(endTime) + ", " + "NULL" + ", " + "NULL" + ") RETURNING id;";
    LOGFile::Warning(sentence.c_str());
    DB->ExecSentenceAsyncResult(sentence);

    if (DB->FecthRow())
    {
        uint32_t cychist_id = DB->ResultUInt64("id");

        for (size_t i = 0; i < cumulativeCount.size(); i++)
        {
            for (size_t j = 0; j < cumulativeCount[i].size(); j++)
            {
                for (size_t k = 0; k < cumulativeCount[i][j].size(); k++)
                {
                    uint64_t value = cumulativeCount[i][j][k];
                    if (value != 0)
                    {
                        DB->ExecSentenceNoResult("INSERT INTO cychist_point (cychist_id, phase, x, y, count) VALUES (" +
                                                 DB->Bind(cychist_id) + ", " + DB->Bind(i) + ", " + DB->Bind(j) + ", " + DB->Bind(k) + ", " + DB->Bind(value) +
                                                 ");");
                    }
                }
            }
        }
    }

    DB->ExecSentenceNoResult("COMMIT TRANSACTION;");
}

void CyclicHistogram::Clear()
{
    startTime = 0;
    cumulativeCount = vector<vector<vector<uint64_t>>>();
}

void CyclicHistogram::ResizePhaseNumber(uint8_t numPhases)
{
    string message = "[CH] Resizing to accomodate " + to_string(numPhases) + " phases.\n";
    LOGFile::Warning(message.c_str());

    // Resize outer vector
    cumulativeCount.resize(numPhases);
}

void CyclicHistogram::ResizeCumulativeCount(uint8_t phaseNumber, size_t newSize)
{
    string message = "[CH] Resizing array from " + to_string(cumulativeCount[phaseNumber].size()) + " to " + to_string(newSize) + "\n";
    LOGFile::Warning(message.c_str());

    // Resize vector
    cumulativeCount[phaseNumber].resize(newSize, vector<uint64_t>(voltageBinSize));
}

vector<vector<vector<uint64_t>>> CyclicHistogram::GetCyclicHistogram()
{
    return cumulativeCount;
}