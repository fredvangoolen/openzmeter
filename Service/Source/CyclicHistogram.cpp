#include "CyclicHistogram.h"
#include <chrono>

CyclicHistogram::CyclicHistogram(const string &analyzer_schema, uint32_t nominalVoltage, size_t voltageBinSize)
{
    this->nominalVoltage = nominalVoltage;
    this->voltageBinSize = voltageBinSize;
    // Setup database structure
    DB = new Database(analyzer_schema);
    DB->ExecResource("SQL/Database_CyclicHistogram.sql");

    skippedFirstPointSet = false;
    startTime = 0;
    endTime = 0;
}

CyclicHistogram::~CyclicHistogram()
{
    // Cleanup memory
    if (DB != NULL)
    {
        delete DB;
    }
    DB = NULL;
}

void CyclicHistogram::AddPoints(float *pVoltageSamples, size_t maxSamplesPerPhase, size_t usableSamplesPerPhase, uint8_t numPhases, float sampleRate, float nominalFrequency)
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
        LOGFile::Debug("[CH] Skipping first set of data.\n");
        skippedFirstPointSet = true;
        return;
    }

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
    for (uint32_t i = 0; i < (usableSamplesPerPhase - 1); i++)
    {
        for (uint8_t phaseNumber = 0; phaseNumber < numPhases; phaseNumber++)
        {
            float voltage_current = pVoltageSamples[(maxSamplesPerPhase * phaseNumber) + i];
            float voltage_next = pVoltageSamples[(maxSamplesPerPhase * phaseNumber) + i + 1]; // Look-ahead to next voltage

            // Look ahead at the next sample. If it also has a value of 0, then don't include any additional samples for this phase (to prevent any discontinuous jumps).+
            /*
            if ((phaseNumber == 0) && (voltage_current == 0.0) && (voltage_next == 0.0))
            {
                LOGFile::Warning("[CH] Skipping future data points due to multiple 0 samples.\n");
                LOGFile::Warning((to_string(phaseNumber) + "\n").c_str());
                LOGFile::Warning((to_string(maxSamplesPerPhase) + "\n").c_str());
                LOGFile::Warning((to_string(usableSamplesPerPhase) + "\n").c_str());
                LOGFile::Warning((to_string(i) + " > " + to_string(voltage_current) + "\n").c_str());
                LOGFile::Warning((to_string(i + 1) + " > " + to_string(voltage_next) + "\n").c_str());
                return;
            }
            else if (abs((voltage_next - voltage_current) > 10))
            {
                // Don't record rapid changes in voltage (i.e. discontinuous, unrealistic jumps in samples)
                LOGFile::Warning("[CH] Skipping future data points due to discontinuous jump in samples.\n");
                LOGFile::Warning((to_string(phaseNumber) + "\n").c_str());
                LOGFile::Warning((to_string(arrBlockSize) + "\n").c_str());
                LOGFile::Warning((to_string(abs((voltage_next - voltage_current) / voltage_current)) + "\n").c_str());
                LOGFile::Warning((to_string(i) + " > " + to_string(voltage_current) + "\n").c_str());
                LOGFile::Warning((to_string(i + 1) + " > " + to_string(voltage_next) + "\n").c_str());
                return;
            }
            */

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

void CyclicHistogram::WriteToDisk()
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

void CyclicHistogram::WriteToDatabase()
{
    WriteToDatabase(0);
}

void CyclicHistogram::WriteToDatabase(uint64_t event_starttime)
{
    if (startTime == 0 && endTime == 0)
    {
        // No data has been added to the CH. Don't store anything in database.
        return;
    }

    DB->ExecSentenceNoResult("BEGIN TRANSACTION;");

    string sentence = "INSERT INTO cychist (starttime, endtime, event_starttime) VALUES (" + DB->Bind(startTime) + ", " + DB->Bind(endTime) + ", " + (event_starttime != 0 ? DB->Bind(event_starttime) : "NULL") + ") RETURNING id;";
    DB->ExecSentenceAsyncResult(sentence);

    if (DB->FecthRow())
    {
        uint32_t cychist_id = DB->ResultUInt64("id");
        string sql_points = "";
        bool shouldExec = false;
        sql_points += "INSERT INTO cychist_point (cychist_id, phase, x, y, count) VALUES ";

        for (size_t i = 0; i < cumulativeCount.size(); i++)
        {
            for (size_t j = 0; j < cumulativeCount[i].size(); j++)
            {
                for (size_t k = 0; k < cumulativeCount[i][j].size(); k++)
                {
                    uint64_t value = cumulativeCount[i][j][k];
                    if (value != 0)
                    {
                        sql_points += "(" + DB->Bind(cychist_id) + "," + DB->Bind(i) + "," + DB->Bind(j) + "," + DB->Bind(k) + "," + DB->Bind(value) + "),";
                        shouldExec = true;
                    }
                }
            }
        }

        if (shouldExec)
        {
            sql_points = sql_points.substr(0, sql_points.size() - 1); // Remove last comma from the SQL statement
            sql_points += ";";
            DB->ExecSentenceNoResult(sql_points);
            DB->ExecSentenceNoResult("COMMIT TRANSACTION;");
        }
        else
        {
            DB->ExecSentenceNoResult("ROLLBACK TRANSACTION;");
        }
    }
}

void CyclicHistogram::Clear()
{
    startTime = 0;
    endTime = 0;
    cumulativeCount = vector<vector<vector<uint64_t>>>();
}

void CyclicHistogram::ResizePhaseNumber(uint8_t numPhases)
{
    string message = "[CH] Resizing to accomodate " + to_string(numPhases) + " phases.\n";
    LOGFile::Debug(message.c_str());

    // Resize outer vector
    cumulativeCount.resize(numPhases);
}

void CyclicHistogram::ResizeCumulativeCount(uint8_t phaseNumber, size_t newSize)
{
    string message = "[CH] Resizing array from " + to_string(cumulativeCount[phaseNumber].size()) + " to " + to_string(newSize) + "\n";
    LOGFile::Debug(message.c_str());

    // Resize vector
    cumulativeCount[phaseNumber].resize(newSize, vector<uint64_t>(voltageBinSize));
}

json CyclicHistogram::GetCyclicHistogramForEvent(uint64_t event_starttime)
{
    json result = json::object();

    DB->ExecSentenceAsyncResult("SELECT id, event_starttime, starttime, endtime FROM cychist WHERE event_starttime = " + DB->Bind(event_starttime));
    if (DB->FecthRow())
    {
        result["id"] = stoi(DB->ResultString("id"));
        result["event_starttime"] = stol(DB->ResultString("event_starttime"));
        result["starttime"] = stol(DB->ResultString("starttime"));
        result["endtime"] = stol(DB->ResultString("endtime"));
    }

    DB->ExecSentenceAsyncResult("SELECT json_build_object('id', COALESCE(ARRAY_AGG(id ORDER BY phase, x, y ASC), '{}'), 'phase', COALESCE(ARRAY_AGG(phase ORDER BY phase, x, y ASC), '{}'), 'sample', COALESCE(ARRAY_AGG(x ORDER BY phase, x, y ASC), '{}'), 'voltage_bin', COALESCE(ARRAY_AGG(y ORDER BY phase, x, y ASC), '{}'), 'count', COALESCE(ARRAY_AGG(count ORDER BY phase, x, y ASC), '{}')) AS points FROM cychist_point WHERE cychist_id = " + DB->ResultString("id"));
    if (DB->FecthRow())
    {
        json points = json::parse(DB->ResultString("points"));

        json points_id = points["id"];
        json points_phase = points["phase"];
        json points_sample = points["sample"];
        json points_voltage_bin = points["voltage_bin"];
        json points_count = points["count"];

        result["points"] = json::array();
        for (size_t i = 0; i < points_id.size(); i++)
        {
            json new_point = json::object();

            new_point["id"] = points_id[i];
            new_point["phase"] = points_phase[i];
            new_point["sample"] = points_sample[i];
            new_point["voltage_bin"] = points_voltage_bin[i];
            new_point["count"] = points_count[i];

            result["points"].push_back(new_point);
        }
    }

    return result;
}

json CyclicHistogram::GetCurrentCyclicHistogram()
{
    json result = json::object();

    result["starttime"] = startTime;
    result["endtime"] = endTime;

    result["points"] = json::array();

    for (size_t i = 0; i < cumulativeCount.size(); i++)
    {
        for (size_t j = 0; j < cumulativeCount[i].size(); j++)
        {
            for (size_t k = 0; k < cumulativeCount[i][j].size(); k++)
            {
                uint64_t count = cumulativeCount[i][j][k];
                if (count != 0)
                {
                    json point = json::object();
                    point["phase"] = i;
                    point["sample"] = j;
                    point["voltage_bin"] = k;
                    point["count"] = count;

                    result["points"].push_back(point);
                }
            }
        }
    }

    return result;
}