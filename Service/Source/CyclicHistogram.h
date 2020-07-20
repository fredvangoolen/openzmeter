#pragma once

#include "Tools.h"
#include "SysSupport.h"
#include <stdlib.h>

class CyclicHistogram final
{
public:
    CyclicHistogram(const string &analyzer_schema, uint32_t nominalVoltage, size_t voltageBinSize = 512);
    ~CyclicHistogram();
    void AddPoints(float *pVoltageSamples, size_t arrBlockSize, uint8_t numPhases, float sampleRate, float nominalFrequency);
    json GetCyclicHistogramForEvent(uint64_t event_starttime);
    json GetCurrentCyclicHistogram();
    void WriteToDisk();
    void WriteToDatabase();
    void WriteToDatabase(uint64_t event_starttime);
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