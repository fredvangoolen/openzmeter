#pragma once

#include "Tools.h"
#include "SysSupport.h"
#include <stdlib.h>

class CyclicHistogram final
{
public:
    CyclicHistogram(const string &analyzer_schema, uint32_t nominalVoltage, size_t voltageBinSize = 512);
    ~CyclicHistogram();
    void ResetCumulativeCount();
    void AddPoints(uint8_t phaseNumber, float *pVoltageSamples, size_t arrSize, size_t offset);
    vector<vector<vector<uint64_t>>> GetCyclicHistogram();
    void FlushToDisk();

private:
    bool skippedFirstPointSet;
    uint32_t nominalVoltage;
    size_t voltageBinSize;
    Database *DB;
    vector<vector<vector<uint64_t>>> cumulativeCount; // Variable used for storing cyclic histogram cumulative counts
    void ResizePhaseNumber(uint8_t numPhases);
    void ResizeCumulativeCount(uint8_t phaseNumber, size_t newSize);
};