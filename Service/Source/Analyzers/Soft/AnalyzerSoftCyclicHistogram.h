#pragma once

#include "Tools.h"
#include "LogFile.h"
#include "Database.h"
#include "AnalyzerSoftBlock.h"
#include <stdlib.h>

class AnalyzerSoftCyclicHistogram final
{
public:
    AnalyzerSoftCyclicHistogram(const string &analyzer_schema, uint32_t nominalVoltage, size_t voltageBinSize = 512);
    ~AnalyzerSoftCyclicHistogram();
    void AddPoints(AnalyzerSoftBlock *inputBlock);
    json GetCyclicHistogramForEvent(uint64_t eventStartTime);
    json GetCurrentCyclicHistogram();
    void WriteToDisk();
    void WriteToDatabase();
    void WriteToDatabase(uint64_t eventStartTime);
    void Clear();

private:
    bool skippedFirstPointSet;
    uint32_t nominalVoltage;
    size_t voltageBinSize;
    Database *DB;
    uint64_t startTime;
    uint64_t endTime;
    vector<vector<vector<uint64_t>>> cumulativeCount; // Variable used for storing cyclic histogram cumulative counts
    void ResizePhaseNumber(uint8_t numPhases);
    void ResizeCumulativeCount(uint8_t phaseNumber, size_t newSize);
};