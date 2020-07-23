#include "JoltAlgorithm.h"
#include <chrono>

JoltAlgorithm::JoltAlgorithm(const string &analyzer_schema)
{
    DB = new Database(analyzer_schema);
    DB->ExecResource("SQL/Database_Jolt.sql");

    ClearStats();
}

JoltAlgorithm::~JoltAlgorithm()
{
    // Cleanup memory
    if (DB != NULL)
    {
        delete DB;
    }
    DB = NULL;
}

vector<float> JoltAlgorithm::ComputeDVDT(vector<float> input)
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

void JoltAlgorithm::AddPoints(float *pVoltageSamples, size_t arrBlockSize, uint8_t numPhases, float sampleRate)
{
    size_t numUsableSamplesPerPhase = min((size_t)sampleRate / 5, arrBlockSize);

    if (endTime - startTime > 300000)
    {
        //LOGFile::Warning("Logging JA data...\n");
        WriteToDatabase();
        ClearStats();
        //LOGFile::Warning("  - Done.\n");
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
        for (size_t i = 0; i < (numUsableSamplesPerPhase - 1); i++)
        {
            float sample = pVoltageSamples[(phaseNumber * arrBlockSize) + i];

            // Look ahead at the next sample. If it also has a value of 0, then don't include any additional samples for this phase (to prevent any discontinuous jumps).
            float sample_next = pVoltageSamples[(phaseNumber * arrBlockSize) + i + 1];
            if (sample == 0.0 && sample_next == 0.0)
            {
                break;
            }
            else
            {
                waveform.push_back(sample);

                // If the last iteration, make sure to include the very last sample
                if (i == (numUsableSamplesPerPhase - 2))
                {
                    waveform.push_back(sample_next);
                }
            }
        }

        // Compute the third-order DVDT of the input waveform
        vector<float> jolt = ComputeDVDT(ComputeDVDT(ComputeDVDT(waveform)));

        for (size_t i = 0; i < jolt.size(); i++)
        {
            float joltElement = jolt[i];

            // Compute the absolute value of the element
            joltElement = abs(joltElement);

            // Update the cumulative stats with new data
            if (stats[phaseNumber].numPoints == 0)
            {
                stats[phaseNumber].min = joltElement;
                stats[phaseNumber].max = joltElement;
                stats[phaseNumber].avg = joltElement;
            }
            else
            {
                if (joltElement < stats[phaseNumber].min)
                {
                    //LOGFile::Warning(("[JA] Found new min: " + to_string(stats[phaseNumber].min) + " -> " + to_string(joltElement) + "\n").c_str());
                    stats[phaseNumber].min = joltElement;
                }

                if (joltElement > stats[phaseNumber].max)
                {
                    //LOGFile::Warning(("[JA] Found new max: " + to_string(stats[phaseNumber].max) + " -> " + to_string(joltElement) + "\n").c_str());
                    stats[phaseNumber].max = joltElement;
                }

                // Calcuate the Cumulative Moving Average (CMA)
                // Algorithm Source: https://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average
                stats[phaseNumber].avg = (joltElement + (stats[phaseNumber].numPoints * stats[phaseNumber].avg)) / (stats[phaseNumber].numPoints + 1);
            }

            ++stats[phaseNumber].numPoints;
        }
    }
}

void JoltAlgorithm::WriteToDatabase()
{
    if (startTime == 0 && endTime == 0)
    {
        // No data has been added to the JA. Don't store anything in database.
        return;
    }

    DB->ExecSentenceNoResult("BEGIN TRANSACTION;");

    for (size_t phaseNumber = 0; phaseNumber < stats.size(); phaseNumber++)
    {
        string sentence = "INSERT INTO jolt (starttime, endtime, phase, num_points, min, max, avg) VALUES (";
        sentence += DB->Bind(startTime) + ",";
        sentence += DB->Bind(endTime) + ",";
        sentence += DB->Bind(phaseNumber) + ",";
        sentence += DB->Bind(stats[phaseNumber].numPoints) + ",";
        sentence += DB->Bind(stats[phaseNumber].min) + ",";
        sentence += DB->Bind(stats[phaseNumber].max) + ",";
        sentence += DB->Bind(stats[phaseNumber].avg) + ");";
        DB->ExecSentenceNoResult(sentence);
    }

    DB->ExecSentenceNoResult("COMMIT TRANSACTION;");
}

void JoltAlgorithm::ClearStats()
{
    stats.clear();
    startTime = 0;
    endTime = 0;
}