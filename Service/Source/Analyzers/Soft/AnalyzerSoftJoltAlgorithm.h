#pragma once

#include "Tools.h"
#include "Database.h"
#include "AnalyzerSoftBlock.h"
#include <stdlib.h>

struct JoltStats
{
    uint64_t numPoints = 0;
    float min = 0.0f;
    float avg = 0.0f;
    float max = 0.0f;
};

class AnalyzerSoftJoltAlgorithm final
{
public:
    AnalyzerSoftJoltAlgorithm(const string &analyzer_schema);
    ~AnalyzerSoftJoltAlgorithm();
    vector<float> ComputeDVDT(vector<float> input);
    void AddPoints(AnalyzerSoftBlock *inputBlock);
    void WriteToDatabase();
    void ClearStats();
    json GetJoltData(uint64_t startTime, uint64_t endTime);

private:
    Database *DB;
    uint64_t startTime;
    uint64_t endTime;
    vector<JoltStats> stats;
};