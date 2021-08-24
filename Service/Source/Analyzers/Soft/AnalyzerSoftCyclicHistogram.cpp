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
    result["VoltageMaxAmplitude"] = pVoltageMaxAmplitude;
    result["CurrentMaxAmplitude"] = pCurrentMaxAmplitude;
    result["VoltageFilterMode"] = pVoltageFilterMode;
    result["CurrentFilterMode"] = pCurrentFilterMode;
    result["VoltageBinCount"] = pVoltageBinCount;
    result["CurrentBinCount"] = pCurrentBinCount;

    return result;
}

void AnalyzerSoftCyclicHistogram::SetConfig(const json &config)
{
    pEnabled = false;
    pVoltageFilterMode = CyclicHistogramVoltageFilterMode::VOLTAGE_FILTER_NONE;
    pCurrentFilterMode = CyclicHistogramCurrentFilterMode::CURRENT_FILTER_NONE;
    pVoltageMaxAmplitude = 120 * 1.7;
    pCurrentMaxAmplitude = 20 * 1.7;
    pVoltageBinCount = 512;
    pCurrentBinCount = 512;

    if (config == NULL)
        return;

    if (config.contains("Enabled") && config["Enabled"].is_boolean())
        pEnabled = config["Enabled"];
    if (config.contains("VoltageFilterMode") && config["VoltageFilterMode"].is_number_unsigned())
        pVoltageFilterMode = config["VoltageFilterMode"];
    if (config.contains("CurrentFilterMode") && config["CurrentFilterMode"].is_number_unsigned())
        pCurrentFilterMode = config["CurrentFilterMode"];
    if (config.contains("VoltageMaxAmplitude") && config["VoltageMaxAmplitude"].is_number())
        pVoltageMaxAmplitude = config["VoltageMaxAmplitude"];
    if (config.contains("CurrentMaxAmplitude") && config["CurrentMaxAmplitude"].is_number())
        pCurrentMaxAmplitude = config["CurrentMaxAmplitude"];
    if (config.contains("VoltageBinCount") && config["VoltageBinCount"].is_number_unsigned())
        pVoltageBinCount = config["VoltageBinCount"];
    if (config.contains("CurrentBinCount") && config["CurrentBinCount"].is_number_unsigned())
        pCurrentBinCount = config["CurrentBinCount"];
}

json AnalyzerSoftCyclicHistogram::CheckConfig(const json &config)
{
    json ConfigOUT = json::object();

    ConfigOUT["Enabled"] = false;
    ConfigOUT["VoltageFilterMode"] = CyclicHistogramVoltageFilterMode::VOLTAGE_FILTER_NONE;
    ConfigOUT["CurrentFilterMode"] = CyclicHistogramCurrentFilterMode::CURRENT_FILTER_NONE;
    ConfigOUT["VoltageMaxAmplitude"] = 120 * 1.7;
    ConfigOUT["CurrentMaxAmplitude"] = 20 * 1.7;
    ConfigOUT["VoltageBinCount"] = 512;
    ConfigOUT["CurrentBinCount"] = 512;

    if (config != NULL && config.is_object())
    {
        if (config.contains("Enabled") && config["Enabled"].is_boolean())
            ConfigOUT["Enabled"] = config["Enabled"];
        if (config.contains("VoltageFilterMode") && config["VoltageFilterMode"].is_number_unsigned())
            ConfigOUT["VoltageFilterMode"] = config["VoltageFilterMode"];
        if (config.contains("CurrentFilterMode") && config["CurrentFilterMode"].is_number_unsigned())
            ConfigOUT["CurrentFilterMode"] = config["CurrentFilterMode"];
        if (config.contains("VoltageMaxAmplitude") && config["VoltageMaxAmplitude"].is_number())
            ConfigOUT["VoltageMaxAmplitude"] = config["VoltageMaxAmplitude"];
        if (config.contains("CurrentMaxAmplitude") && config["CurrentMaxAmplitude"].is_number())
            ConfigOUT["CurrentMaxAmplitude"] = config["CurrentMaxAmplitude"];
        if (config.contains("VoltageBinCount") && config["VoltageBinCount"].is_number_unsigned())
            ConfigOUT["VoltageBinCount"] = config["VoltageBinCount"];
        if (config.contains("CurrentBinCount") && config["CurrentBinCount"].is_number_unsigned())
            ConfigOUT["CurrentBinCount"] = config["CurrentBinCount"];
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
    uint8_t numCurrentChannels = inputBlock->CurrentChannels();
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

    ResizeVoltageHistogram(numVoltageChannels, numNominalSamplesPerCycle);
    ResizeCurrentHistogram(numCurrentChannels, numNominalSamplesPerCycle);

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

                if (this->pVoltageFilterMode == CyclicHistogramVoltageFilterMode::VOLTAGE_FILTER_IDEAL)
                {
                    float t = (float)sampleIdx / (float)numNominalSamplesPerCycle;

                    float idealVoltage = nominalVoltage * sqrt(2) * sin(2 * M_PI * t - (2 * M_PI * phaseNumber / numVoltageChannels));
                    voltageNew = voltageNew - idealVoltage;
                }
                else if (this->pVoltageFilterMode == CyclicHistogramVoltageFilterMode::VOLTAGE_FILTER_NOMINAL)
                {
                    if (!nominalSamplesSaved)
                    {
                        if (sampleIdx < numNominalSamplesPerCycle)
                        {
                            nomVoltageSamples[phaseNumber][sampleIdx] += voltageNew;
                            nomVoltageSamplesCount[phaseNumber][sampleIdx] += 1;
                        }
                        continue;
                    }
                    else
                    {
                        // Apply the nominal voltage filter
                        voltageNew = voltageNew - nomVoltageSamples[phaseNumber][sampleIdx];
                    }
                }

                // Don't attempt to add to cumulative count if the voltage is clipped
                if (abs(voltageNew) <= pVoltageMaxAmplitude)
                {
                    // Convert voltage measurement into a discrete bin number (i.e. quantization)
                    uint32_t binIndex = ((voltageNew + pVoltageMaxAmplitude) / (2 * pVoltageMaxAmplitude)) * (pVoltageBinCount - 1);

                    if (sampleIdx < numNominalSamplesPerCycle)
                    {
                        if (phaseNumber == 0 && sampleIdx == 0)
                            continue;

                        // Perform the incremental count update
                        voltageHistogram[phaseNumber][sampleIdx][binIndex] += 1;
                    }
                }
            }

            // Current CH
            float current_sample = inputBlock->CurrentSample(phaseNumber, i);
            if (abs(current_sample) <= pCurrentMaxAmplitude)
            {
                uint32_t binIndex = ((current_sample + pCurrentMaxAmplitude) / (2 * pCurrentMaxAmplitude)) * (pCurrentBinCount - 1);
                currentHistogram[phaseNumber][i % numNominalSamplesPerCycle][binIndex] += 1;
            }
        }
    }

    if (pVoltageFilterMode == CyclicHistogramVoltageFilterMode::VOLTAGE_FILTER_NOMINAL && !nominalSamplesSaved)
    {
        // Compute the average for each sample
        for (uint8_t phaseNumber = 0; phaseNumber < numVoltageChannels; phaseNumber++)
        {
            for (uint64_t i = 0; i < nomVoltageSamples[phaseNumber].size(); i++)
            {
                nomVoltageSamples[phaseNumber][i] = nomVoltageSamples[phaseNumber][i] / nomVoltageSamplesCount[phaseNumber][i];
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

    for (size_t i = 0; i < voltageHistogram.size(); i++)
    {
        for (size_t j = 0; j < voltageHistogram[i].size(); j++)
        {
            for (size_t k = 0; k < voltageHistogram[i][j].size(); k++)
            {
                uint64_t value = voltageHistogram[i][j][k];
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

        for (size_t i = 0; i < voltageHistogram.size(); i++)
        {
            for (size_t j = 0; j < voltageHistogram[i].size(); j++)
            {
                for (size_t k = 0; k < voltageHistogram[i][j].size(); k++)
                {
                    uint64_t value = voltageHistogram[i][j][k];
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
    voltageHistogram = vector<vector<vector<uint64_t>>>();
    nomVoltageSamples = vector<vector<float>>();
    nomVoltageSamplesCount = vector<vector<uint64_t>>();
    nominalSamplesSaved = false;
}

void AnalyzerSoftCyclicHistogram::ResizeVoltageHistogram(uint8_t numVoltageChannels, size_t numSamples)
{
    // Check if vector is large enough for number of phases
    if (numVoltageChannels > voltageHistogram.size())
        voltageHistogram.resize(numVoltageChannels);

    if (numVoltageChannels > nomVoltageSamples.size())
        nomVoltageSamples.resize(numVoltageChannels);

    if (numVoltageChannels > nomVoltageSamplesCount.size())
        nomVoltageSamplesCount.resize(numVoltageChannels);

    // Check if inner vectors are large enough to hold individual phase's samples
    for (uint8_t i = 0; i < numVoltageChannels; i++)
    {
        // Resize the cumulative count array to fit all data
        if (numSamples > voltageHistogram[i].size())
            voltageHistogram[i].resize(numSamples, vector<uint64_t>(pVoltageBinCount));

        if (numSamples > nomVoltageSamples[i].size())
            nomVoltageSamples[i].resize(numSamples, 0);

        if (numSamples > nomVoltageSamplesCount[i].size())
            nomVoltageSamplesCount[i].resize(numSamples, 0);
    }
}

void AnalyzerSoftCyclicHistogram::ResizeCurrentHistogram(uint8_t numCurrentChannels, size_t numSamples)
{
    // Check if vector is large enough for number of channels
    if (numCurrentChannels > currentHistogram.size())
        currentHistogram.resize(numCurrentChannels);

    if (numCurrentChannels > nomCurrentSamples.size())
        nomCurrentSamples.resize(numCurrentChannels);

    if (numCurrentChannels > nomCurrentSamplesCount.size())
        nomCurrentSamplesCount.resize(numCurrentChannels);

    // Check if inner vectors are large enough to hold individual channel's samples
    for (uint8_t i = 0; i < numCurrentChannels; i++)
    {
        // Resize the cumulative count array to fit all data
        if (numSamples > currentHistogram[i].size())
            currentHistogram[i].resize(numSamples, vector<uint64_t>(pCurrentBinCount));

        if (numSamples > nomCurrentSamples[i].size())
            nomCurrentSamples[i].resize(numSamples, 0);

        if (numSamples > nomCurrentSamplesCount[i].size())
            nomCurrentSamplesCount[i].resize(numSamples, 0);
    }
}

json AnalyzerSoftCyclicHistogram::GetCyclicHistogramForEvent(string eventStartTime)
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
        json points_channel = points["phase"];
        json points_sample = points["sample"];
        json points_voltage_bin = points["voltage_bin"];
        json points_count = points["count"];

        // Add voltage CH data
        result["voltagePoints"] = json::object();
        result["voltagePoints"]["channel"] = json::array();
        result["voltagePoints"]["x"] = json::array();
        result["voltagePoints"]["y"] = json::array();
        result["voltagePoints"]["z"] = json::array();

        for (size_t i = 0; i < points_id.size(); i++)
        {

            result["voltagePoints"]["channel"].push_back(points_channel[i]);
            result["voltagePoints"]["x"].push_back(points_sample[i]);
            result["voltagePoints"]["y"].push_back(points_voltage_bin[i]);
            result["voltagePoints"]["z"].push_back(points_count[i]);

        }
    }

    return result;
}

json AnalyzerSoftCyclicHistogram::GetCurrentCyclicHistogram(string mode)
{
    json result = json::object();

    result["starttime"] = startTime;
    result["endtime"] = endTime;

    if (mode == "voltage")
    {
        // Add voltage CH data
        result["voltagePoints"] = json::object();
        result["voltagePoints"]["channel"] = json::array();
        result["voltagePoints"]["x"] = json::array();
        result["voltagePoints"]["y"] = json::array();
        result["voltagePoints"]["z"] = json::array();

        for (size_t i = 0; i < voltageHistogram.size(); i++)
        {
            for (size_t j = 0; j < voltageHistogram[i].size(); j++)
            {
                for (size_t k = 0; k < voltageHistogram[i][j].size(); k++)
                {
                    uint64_t count = voltageHistogram[i][j][k];
                    if (count != 0)
                    {
                        result["voltagePoints"]["channel"].push_back(i);
                        result["voltagePoints"]["x"].push_back(j);
                        result["voltagePoints"]["y"].push_back(k);
                        result["voltagePoints"]["z"].push_back(count);
                    }
                }
            }
        }
    }
    else if (mode == "current")
    {
        // Add current CH data
        result["currentPoints"] = json::object();
        result["currentPoints"]["channel"] = json::array();
        result["currentPoints"]["x"] = json::array();
        result["currentPoints"]["y"] = json::array();
        result["currentPoints"]["z"] = json::array();

        for (size_t i = 0; i < currentHistogram.size(); i++)
        {
            for (size_t j = 0; j < currentHistogram[i].size(); j++)
            {
                for (size_t k = 0; k < currentHistogram[i][j].size(); k++)
                {
                    uint64_t count = currentHistogram[i][j][k];
                    if (count != 0)
                    {
                        result["currentPoints"]["channel"].push_back(i);
                        result["currentPoints"]["x"].push_back(j);
                        result["currentPoints"]["y"].push_back(k);
                        result["currentPoints"]["z"].push_back(count);
                    }
                }
            }
        }
    }

    return result;
}