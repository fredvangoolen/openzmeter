#include "AnalyzerSoftJoltAlgorithm.h"
#include <chrono>

AnalyzerSoftJoltAlgorithm::AnalyzerSoftJoltAlgorithm(const string &analyzer_schema)
{
    DB = new Database(analyzer_schema);
    DB->ExecResource("SQL/Database_Jolt.sql");

    ClearStats();
}

AnalyzerSoftJoltAlgorithm::~AnalyzerSoftJoltAlgorithm()
{
    // Cleanup memory
    if (DB != NULL)
    {
        delete DB;
    }
    DB = NULL;
}

vector<float> AnalyzerSoftJoltAlgorithm::ComputeDVDT(vector<float> input)
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

void AnalyzerSoftJoltAlgorithm::AddPoints(AnalyzerSoftBlock *inputBlock)
{
    size_t usableSamplesPerPhase = inputBlock->Samples();
    uint8_t numPhases = inputBlock->VoltageChannels();

    if (endTime - startTime > 60000)
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
        for (size_t i = 0; i < (usableSamplesPerPhase - 1); i++)
        {
            float sample = inputBlock->VoltageSample(phaseNumber, i);

            // Look ahead at the next sample. If it also has a value of 0, then don't include any additional samples for this phase (to prevent any discontinuous jumps).
            float sample_next = inputBlock->VoltageSample(phaseNumber, i + 1);
            if (sample == 0.0 && sample_next == 0.0)
            {
                break;
            }
            else
            {
                waveform.push_back(sample);

                // If the last iteration, make sure to include the very last sample
                if (i == (usableSamplesPerPhase - 2))
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

void AnalyzerSoftJoltAlgorithm::WriteToDatabase()
{
    if (startTime == 0 && endTime == 0)
    {
        // No data has been added to the JA. Don't store anything in database.
        return;
    }

    DB->ExecSentenceNoResult("BEGIN TRANSACTION;");

    for (size_t phaseNumber = 0; phaseNumber < stats.size(); phaseNumber++)
    {
        string sentence = "INSERT INTO jolt (time, phase, num_points, min, max, avg) VALUES (";
        sentence += DB->Bind(endTime) + ","; // Use the ending time as the 'official' time when storing data
        sentence += DB->Bind(phaseNumber) + ",";
        sentence += DB->Bind(stats[phaseNumber].numPoints) + ",";
        sentence += DB->Bind(stats[phaseNumber].min) + ",";
        sentence += DB->Bind(stats[phaseNumber].max) + ",";
        sentence += DB->Bind(stats[phaseNumber].avg) + ");";
        DB->ExecSentenceNoResult(sentence);
    }

    DB->ExecSentenceNoResult("COMMIT TRANSACTION;");
}

void AnalyzerSoftJoltAlgorithm::ClearStats()
{
    stats.clear();
    startTime = 0;
    endTime = 0;
}

json AnalyzerSoftJoltAlgorithm::GetJoltData(uint64_t startTime, uint64_t endTime)
{
    json result = json::object();
    result["starttime"] = startTime;
    result["endtime"] = endTime;

    string sentence = "SELECT json_build_object(";
    sentence += "'id', COALESCE(ARRAY_AGG(id ORDER BY time, phase ASC), '{}'),";
    sentence += "'time', COALESCE(ARRAY_AGG(time ORDER BY time, phase ASC), '{}'),";
    sentence += "'phase', COALESCE(ARRAY_AGG(phase ORDER BY time, phase ASC), '{}'),";
    sentence += "'num_points', COALESCE(ARRAY_AGG(num_points ORDER BY time, phase ASC), '{}'),";
    sentence += "'min', COALESCE(ARRAY_AGG(min ORDER BY time, phase ASC), '{}'),";
    sentence += "'max', COALESCE(ARRAY_AGG(max ORDER BY time, phase ASC), '{}'),";
    sentence += "'avg', COALESCE(ARRAY_AGG(avg ORDER BY time, phase ASC), '{}')";
    sentence += ") AS points FROM jolt WHERE ";
    sentence += "time >= " + DB->Bind(startTime);
    sentence += " AND time <= " + DB->Bind(endTime);

    DB->ExecSentenceAsyncResult(sentence);
    if (DB->FecthRow())
    {
        json points = json::parse(DB->ResultString("points"));

        json points_id = points["id"];
        json points_time = points["time"];
        json points_phase = points["phase"];
        json points_numPoints = points["num_points"];
        json points_min = points["min"];
        json points_max = points["max"];
        json points_avg = points["avg"];

        result["points"] = json::array();
        for (size_t i = 0; i < points_id.size(); i++)
        {
            json new_point = json::object();

            new_point["id"] = points_id[i];
            new_point["time"] = points_time[i];
            new_point["phase"] = points_phase[i];
            new_point["num_points"] = points_numPoints[i];
            new_point["min"] = points_min[i];
            new_point["max"] = points_max[i];
            new_point["avg"] = points_avg[i];

            result["points"].push_back(new_point);
        }
    }

    return result;
}