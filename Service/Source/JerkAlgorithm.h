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
    JerkAlgorithm(const string &analyzer_schema);
    ~JerkAlgorithm();
    vector<float> ComputeDVDT(vector<float> input);
    void AddPoints(float *pVoltageSamples, size_t arrBlockSize, uint8_t numPhases, float sampleRate);
    void WriteToDatabase();
    void WriteToDatabase(uint64_t event_starttime);
    void ClearStats();

private:
    bool flag;
    Database *DB;
    uint64_t startTime;
    uint64_t endTime;
    vector<JerkStats> stats;
};