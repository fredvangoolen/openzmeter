#pragma once

#include "Tools.h"
#include "SysSupport.h"
#include <stdlib.h>

struct JerkStats{
    uint64_t numPoints;
    float min;
    float avg;
    float max;
}

class JerkAlgorithm final
{
public:
    JerkAlgorithm();
    vector<float> ComputeDVDT(vector<float> input);
    void AddWaveform(vector<float> waveform);
    void ClearStats();
private:
    JerkStats stats;
};