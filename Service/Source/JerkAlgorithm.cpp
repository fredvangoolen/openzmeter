#include "JerkAlgorithm.h"
#include <chrono>

JerkAlgorithm::JerkAlgorithm(const string &analyzer_schema)
{
    DB = new Database(analyzer_schema);
    DB->ExecResource("SQL/Database_Jerk.sql");

    ClearStats();
}

JerkAlgorithm::~JerkAlgorithm()
{
    // Cleanup memory
    if (DB != NULL)
    {
        delete DB;
    }
    DB = NULL;
}

vector<float> JerkAlgorithm::ComputeDVDT(vector<float> input)
{
    vector<float> result = vector<float>();

    for (size_t i = 1; i < input.size(); i++)
    {
        float e1 = input[i - 1];
        float e2 = input[i];
        result.push_back(e2 - e1);
    }

    return result;
}

void JerkAlgorithm::AddPoints(float *pVoltageSamples, size_t arrBlockSize, uint8_t numPhases)
{
    if (endTime - startTime > 120000)
    {
        LOGFile::Warning("Logging JA data...\n");
        WriteToDatabase();
        ClearStats();
        LOGFile::Warning("  - Done.\n");
        return;
    }

    // Log the current time as the end time for this histogram
    endTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if (startTime == 0)
    {
        // Log the current time as the start time for this histogram
        startTime = endTime;
    }

    if (stats.size() < numPhases)
    {
        stats.resize(numPhases);
    }

    for (uint8_t phaseNumber = 0; phaseNumber < numPhases; phaseNumber++)
    {
        // Convert float pointer array to vector of floats
        vector<float> waveform;
        for (size_t i = 0; i < arrBlockSize; i++)
        {
            waveform.push_back(pVoltageSamples[(phaseNumber * arrBlockSize) + i]);
        }

        // Compute the third-order DVDT of the input waveform
        vector<float> jerk = ComputeDVDT(ComputeDVDT(ComputeDVDT(waveform)));

        for (size_t i = 0; i < jerk.size(); i++)
        {
            float jerkElement = jerk[i];

            // Update the cumulative stats with new data
            if (stats[phaseNumber].numPoints == 0)
            {
                stats[phaseNumber].min = jerkElement;
                stats[phaseNumber].max = jerkElement;
                stats[phaseNumber].avg = jerkElement;
            }
            else
            {
                if (jerkElement < stats[phaseNumber].min)
                {
                    LOGFile::Warning(("[JA] Found new min: " + stats[phaseNumber].min + " -> " + jerkElement + "\n").c_str());
                    stats[phaseNumber].min = jerkElement;
                }

                if (jerkElement > stats[phaseNumber].max)
                {
                    LOGFile::Warning(("[JA] Found new max: " + stats[phaseNumber].max + " -> " + jerkElement + "\n").c_str());
                    stats[phaseNumber].max = jerkElement;
                }

                // Calcuate the Cumulative Moving Average (CMA)
                // Algorithm Source: https://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average
                stats[phaseNumber].avg = (jerkElement + (stats[phaseNumber].numPoints * stats[phaseNumber].avg)) / (stats[phaseNumber].numPoints + 1);
            }

            ++stats[phaseNumber].numPoints;
        }
    }
}

void JerkAlgorithm::WriteToDatabase()
{
    WriteToDatabase(0);
}

void JerkAlgorithm::WriteToDatabase(uint64_t event_starttime)
{
    if (startTime == 0 && endTime == 0)
    {
        // No data has been added to the CH. Don't store anything in database.
        return;
    }

    DB->ExecSentenceNoResult("BEGIN TRANSACTION;");

    for (size_t phaseNumber = 0; phaseNumber < stats.size(); phaseNumber++)
    {
        string sentence = "INSERT INTO jerk (starttime, endtime, event_starttime, phase, num_points, min, max, avg) VALUES (";
        sentence += DB->Bind(startTime) + ",";
        sentence += DB->Bind(endTime) + ",";
        sentence += (event_starttime != 0 ? DB->Bind(event_starttime) : "NULL") + ",";
        sentence += DB->Bind(phaseNumber) + ",";
        sentence += DB->Bind(stats[phaseNumber].numPoints) + ",";
        sentence += DB->Bind(stats[phaseNumber].min) + ",";
        sentence += DB->Bind(stats[phaseNumber].max) + ",";
        sentence += DB->Bind(stats[phaseNumber].avg) + ");";
        DB->ExecSentenceNoResult(sentence);
    }

    DB->ExecSentenceNoResult("COMMIT TRANSACTION;");
}

void JerkAlgorithm::ClearStats()
{
    stats.clear();
    startTime = 0;
    endTime = 0;
}