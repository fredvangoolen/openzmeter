#include "JerkAlgorithm.h"
#include <chrono>

JerkAlgorithm::JerkAlgorithm()
{
    ClearStats();
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
                    stats[phaseNumber].min = jerkElement;

                if (jerkElement > stats[phaseNumber].max)
                    stats[phaseNumber].max = jerkElement;

                // Calcuate the Cumulative Moving Average (CMA)
                // Algorithm Source: https://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average
                stats[phaseNumber].avg = (jerkElement + (stats[phaseNumber].numPoints * stats[phaseNumber].avg)) / (stats[phaseNumber].numPoints + 1);
            }

            ++stats[phaseNumber].numPoints;
        }
    }
}

void JerkAlgorithm::ClearStats()
{
    stats.clear();
}