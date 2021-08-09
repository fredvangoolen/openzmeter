#include "AnalyzerSoftCyclicHistogram.h"
#include "ConfigFile.h"
#include <chrono>
#include <fstream>

AnalyzerSoftCyclicHistogram::AnalyzerSoftCyclicHistogram(const string &analyzer_schema)
{
    // Setup database structure
    DB = new Database(analyzer_schema);
    DB->ExecResource("SQL/Database_CyclicHistogram.sql");

    nominalSamplesSaved = false;
    skippedFirstPointSet = false;
    startTime = 0;
    endTime = 0;
}

AnalyzerSoftCyclicHistogram::~AnalyzerSoftCyclicHistogram()
{
    // Cleanup memory
    if (DB != NULL)
    {
        delete DB;
    }
    DB = NULL;
}

json AnalyzerSoftCyclicHistogram::GetConfig()
{
    json result = json::object();

    result["Enabled"] = pEnabled;
    result["MaxAmplitude"] = pMaxAmplitude;
    result["BinCount"] = pBinCount;

    return result;
}

void AnalyzerSoftCyclicHistogram::SetConfig(const json &config)
{
    pEnabled = false;
    pVoltageFilterMode = CyclicHistogramFilterMode::None;
    pMaxAmplitude = 120 * 1.7;
    pBinCount = 512;

    if (config == NULL)
        return;

    if (config.contains("Enabled") && config["Enabled"].is_boolean())
        pEnabled = config["Enabled"];
    if (config.contains("VoltageFilterMode") && config["VoltageFilterMode"].is_number_unsigned())
        pVoltageFilterMode = config["VoltageFilterMode"];
    if (config.contains("MaxAmplitude") && config["MaxAmplitude"].is_number())
        pMaxAmplitude = config["MaxAmplitude"];
    if (config.contains("BinCount") && config["BinCount"].is_number_unsigned())
        pBinCount = config["BinCount"];
}

json AnalyzerSoftCyclicHistogram::CheckConfig(const json &config)
{
    json ConfigOUT = json::object();

    ConfigOUT["Enabled"] = false;
    ConfigOUT["VoltageFilterMode"] = CyclicHistogramFilterMode::None;
    ConfigOUT["MaxAmplitude"] = 120 * 1.7;
    ConfigOUT["BinCount"] = 512;

    if (config != NULL && config.is_object())
    {
        if (config.contains("Enabled") && config["Enabled"].is_boolean())
            ConfigOUT["Enabled"] = config["Enabled"];
        if (config.contains("VoltageFilterMode") && config["VoltageFilterMode"].is_number_unsigned())
            ConfigOUT["VoltageFilterMode"] = config["VoltageFilterMode"];
        if (config.contains("MaxAmplitude") && config["MaxAmplitude"].is_number())
            ConfigOUT["MaxAmplitude"] = config["MaxAmplitude"];
        if (config.contains("BinCount") && config["BinCount"].is_number_unsigned())
            ConfigOUT["BinCount"] = config["BinCount"];
    }

    return ConfigOUT;
}

void AnalyzerSoftCyclicHistogram::AddPoints(AnalyzerSoftBlock *inputBlock, int sampleRate)
{
    if (!pEnabled)
        return;

    int nominalFrequency = (int)inputBlock->NominalFrequency();
    float nominalVoltage = inputBlock->NominalVoltage();
    uint16_t numSamples = inputBlock->Samples();
    uint8_t numVoltageChannels = inputBlock->VoltageChannels();
    int numNominalSamplesPerCycle = sampleRate / nominalFrequency;

    // Log the current time as the end time for this histogram
    endTime = Tools::GetTime64();

    if (startTime == 0)
    {
        // Log the current time as the start time for this histogram
        startTime = endTime;
    }

    // Skip the first requested set of data points (tends to be unreliable data)
    if (!skippedFirstPointSet)
    {
        LogFile::Debug("[CH] Skipping first set of data.\n");
        skippedFirstPointSet = true;
        return;
    }

    // Check if vector is large enough for number of phases
    if (numVoltageChannels > cumulativeCount.size())
    {
        ResizePhaseNumber(numVoltageChannels);
    }

    // Check if inner vectors are large enough to hold individual phase's samples
    for (uint8_t phaseNumber = 0; phaseNumber < numVoltageChannels; phaseNumber++)
    {
        if ((size_t)numNominalSamplesPerCycle > cumulativeCount[phaseNumber].size())
        {
            // Resize the cumulative count array to fit all data
            ResizeCumulativeCount(phaseNumber, numNominalSamplesPerCycle);
        }
    }

    bool foundFirstCycle = false;
    uint32_t cycleStartIdx = 0;
    float cycleXDiff = 0;
    // Find the index of each zero-crossing instance (from - to +)
    for (uint16_t i = 0; i < (numSamples - 1); i++)
    {
        for (uint8_t phaseNumber = 0; phaseNumber < numVoltageChannels; phaseNumber++)
        {
            float voltage_current = inputBlock->VoltageSample(phaseNumber, i);
            float voltage_next = inputBlock->VoltageSample(phaseNumber, i + 1);

            // Look ahead at the next sample. If it also has a value of 0, then don't include any additional samples for this phase (to prevent any discontinuous jumps).
            /*
            if ((phaseNumber == 0) && (voltage_current == 0.0) && (voltage_next == 0.0))
            {
                LogFile::Warning("[CH] Skipping future data points due to multiple 0 samples.\n");
                LogFile::Warning((to_string(phaseNumber) + "\n").c_str());
                LogFile::Warning((to_string(maxSamplesPerPhase) + "\n").c_str());
                LogFile::Warning((to_string(usableSamplesPerPhase) + "\n").c_str());
                LogFile::Warning((to_string(i) + " > " + to_string(voltage_current) + "\n").c_str());
                LogFile::Warning((to_string(i + 1) + " > " + to_string(voltage_next) + "\n").c_str());
                return;
            }
            else if (abs((voltage_next - voltage_current) > 10))
            {
                // Don't record rapid changes in voltage (i.e. discontinuous, unrealistic jumps in samples)
                LogFile::Warning("[CH] Skipping future data points due to discontinuous jump in samples.\n");
                LogFile::Warning((to_string(phaseNumber) + "\n").c_str());
                LogFile::Warning((to_string(arrBlockSize) + "\n").c_str());
                LogFile::Warning((to_string(abs((voltage_next - voltage_current) / voltage_current)) + "\n").c_str());
                LogFile::Warning((to_string(i) + " > " + to_string(voltage_current) + "\n").c_str());
                LogFile::Warning((to_string(i + 1) + " > " + to_string(voltage_next) + "\n").c_str());
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
                int sampleIdx = i - cycleStartIdx;
                float voltageNew = (m * (i + cycleXDiff)) + b;

                if (this->pVoltageFilterMode == CyclicHistogramFilterMode::Ideal)
                {
                    float t = (float)sampleIdx / (float)numNominalSamplesPerCycle;

                    float idealVoltage = nominalVoltage * sqrt(2) * sin(2 * M_PI * t - (2 * M_PI * phaseNumber / numVoltageChannels));
                    // LogFile::Warning("Voltage: %f, Ideal Voltage: %f, Diff: %f", voltageNew, idealVoltage, voltageNew - idealVoltage);
                    voltageNew = voltageNew - idealVoltage;
                }
                else if (this->pVoltageFilterMode == CyclicHistogramFilterMode::Nominal)
                {
                    if (!nominalSamplesSaved)
                    {
                        if (sampleIdx < numNominalSamplesPerCycle)
                        {
                            // LogFile::Warning("[CH] Saving Phase %u, Sample %i, Voltage %f", phaseNumber, sampleIdx, voltageNew);
                            nominalSamples[phaseNumber][sampleIdx] += voltageNew;
                            nominalSamplesCount[phaseNumber][sampleIdx] += 1;
                        }
                        continue;
                    }
                    else
                    {
                        // Apply the nominal voltage filter
                        voltageNew = voltageNew - nominalSamples[phaseNumber][sampleIdx];
                    }
                }

                // Don't attempt to add to cumulative count if the voltage is clipped
                if (abs(voltageNew) <= pMaxAmplitude)
                {
                    // Convert voltage measurement into a discrete bin number (i.e. quantization)
                    uint32_t binIndex = ((voltageNew + pMaxAmplitude) / (2 * pMaxAmplitude)) * (pBinCount - 1);

                    if (sampleIdx < numNominalSamplesPerCycle)
                    {
                        if (phaseNumber == 0 && sampleIdx == 0)
                            continue;

                        // Perform the incremental count update
                        cumulativeCount[phaseNumber][sampleIdx][binIndex] += 1;
                    }
                }
            }
        }
    }

    if (pVoltageFilterMode == CyclicHistogramFilterMode::Nominal && !nominalSamplesSaved)
    {
        // Compute the average for each sample
        for (uint8_t phaseNumber = 0; phaseNumber < numVoltageChannels; phaseNumber++)
        {
            for (uint64_t i = 0; i < nominalSamples[phaseNumber].size(); i++)
            {
                nominalSamples[phaseNumber][i] = nominalSamples[phaseNumber][i] / nominalSamplesCount[phaseNumber][i];
            }
        }

        nominalSamplesSaved = true;
    }
}

void AnalyzerSoftCyclicHistogram::WriteToDisk()
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

void AnalyzerSoftCyclicHistogram::WriteToDatabase()
{
    WriteToDatabase(0);
}

void AnalyzerSoftCyclicHistogram::WriteToDatabase(uint64_t eventStartTime)
{
    if (startTime == 0 && endTime == 0)
    {
        // No data has been added to the CH. Don't store anything in database.
        return;
    }

    DB->ExecSentenceNoResult("BEGIN TRANSACTION;");

    string sentence = "INSERT INTO cychist (starttime, endtime, event_id) VALUES (" + DB->Bind(startTime) + ", " + DB->Bind(endTime) + ", " + (eventStartTime != 0 ? DB->Bind(eventStartTime) : "NULL") + ") RETURNING id;";
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

void AnalyzerSoftCyclicHistogram::Clear()
{
    startTime = 0;
    endTime = 0;
    cumulativeCount = vector<vector<vector<uint64_t>>>();
    nominalSamples = vector<vector<float>>();
    nominalSamplesCount = vector<vector<uint64_t>>();
    nominalSamplesSaved = false;
}

void AnalyzerSoftCyclicHistogram::ResizePhaseNumber(uint8_t numPhases)
{
    string message = "[CH] Resizing to accomodate " + to_string(numPhases) + " phases.\n";
    LogFile::Debug(message.c_str());

    // Resize outer vector
    cumulativeCount.resize(numPhases);

    // Resize nominal filter vectors
    nominalSamples.resize(numPhases);
    nominalSamplesCount.resize(numPhases);
}

void AnalyzerSoftCyclicHistogram::ResizeCumulativeCount(uint8_t phaseNumber, size_t newSize)
{
    string message = "[CH] Resizing array from " + to_string(cumulativeCount[phaseNumber].size()) + " to " + to_string(newSize) + "\n";
    LogFile::Debug(message.c_str());

    // Resize vector
    cumulativeCount[phaseNumber].resize(newSize, vector<uint64_t>(pBinCount));

    nominalSamples[phaseNumber].resize(newSize, 0);
    nominalSamplesCount[phaseNumber].resize(newSize, 0);
}

json AnalyzerSoftCyclicHistogram::GetCyclicHistogramForEvent(uint64_t eventStartTime)
{
    json result = json::object();

    DB->ExecSentenceAsyncResult("SELECT id, event_id, starttime, endtime FROM cychist WHERE event_id = " + DB->Bind(eventStartTime));
    if (DB->FecthRow())
    {
        result["id"] = stoi(DB->ResultString("id"));
        result["event_id"] = DB->ResultString("event_id");
        result["starttime"] = stoll(DB->ResultString("starttime"));
        result["endtime"] = stoll(DB->ResultString("endtime"));
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

json AnalyzerSoftCyclicHistogram::GetCurrentCyclicHistogram()
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
                    json point = json::array();
                    point.push_back(i);     // Phase
                    point.push_back(j);     // Sample
                    point.push_back(k);     // Voltage Bin
                    point.push_back(count); // Count

                    result["points"].push_back(point);
                }
            }
        }
    }

    return result;
}