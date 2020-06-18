#pragma once

#include "Tools.h"
#include "SysSupport.h"
#include <stdlib.h>

struct JerkStats
{
    uint64_t numPoints = 0;
    float min = 0.0f;
    float avg = 0.0f;
    float max = 0.0f;
};

class JerkAlgorithm final
{
public:
    JerkAlgorithm();
    vector<float> ComputeDVDT(vector<float> input);
    void AddPoints(float *pVoltageSamples, size_t arrBlockSize, uint8_t numPhases);
    void ClearStats();

private:
    vector<JerkStats> stats;
};