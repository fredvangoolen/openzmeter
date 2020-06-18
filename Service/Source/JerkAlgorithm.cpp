#include "JerkAlgorithm.h"
#include <chrono>

JerkAlgorithm::JerkAlgorithm()
{
    stats = JerkStats();
    ClearStats();
}

vector<float> JerkAlgorithm::ComputeDVDT(vector<float> input)
{
    vector<float> result = vector<float>();

    for (uint32_t i = 1; i < input.size(); i++)
    {
        float e1 = input[i - 1];
        float e2 = input[i];
        result.push_back(e2 - e1);
    }

    return result;
}

void JerkAlgorithm::AddWaveform(vector<float> waveform)
{
    // Compute the third-order DVDT of the input waveform
    vector<float> jerk = ComputeDVDT(ComputeDVDT(ComputeDVDT(waveform)));

    for (int i = 0; i < jerk.size(); i++)
    {
        float jerkElement = jerk[i];

        // Update the cumulative stats with new data
        if (stats.numPoints == 0)
        {
            stats.min = jerkElement;
            stats.max = jerkElement;
            stats.avg = jerkElement;
        }
        else
        {
            if (jerkElement < stats.min)
                stats.min = jerkElement;

            if (jerkElement > stats.max)
                stats.max = jerkElement;

            // Calcuate the Cumulative Moving Average (CMA)
            // Source: https://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average
            stats.avg = (jerkElement + (stats.numPoints * stats.avg)) / (stats.numPoints + 1);
        }

        ++stats.numPoints;
    }
}

void JerkAlgorithm::ClearStats()
{
    stats.numPoints = 0;
    stats.min = 0;
    stats.avg = 0;
    stats.max = 0;
}