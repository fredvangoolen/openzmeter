#pragma once

#include "Tools.h"
#include "SysSupport.h"
#include <stdlib.h>

struct JoltStats
{
    uint64_t numPoints = 0;
    float min = 0.0f;
    float avg = 0.0f;
    float max = 0.0f;
};

class JoltAlgorithm final
{
public:
    JoltAlgorithm(const string &analyzer_schema);
    ~JoltAlgorithm();
    vector<float> ComputeDVDT(vector<float> input);
    void AddPoints(float *pVoltageSamples, size_t arrBlockSize, uint8_t numPhases, float sampleRate);
    void WriteToDatabase();
    void ClearStats();

private:
    Database *DB;
    uint64_t startTime;
    uint64_t endTime;
    vector<JoltStats> stats;
};