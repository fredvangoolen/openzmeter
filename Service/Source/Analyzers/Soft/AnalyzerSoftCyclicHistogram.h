#pragma once

#include "Tools.h"
#include "LogFile.h"
#include "Database.h"
#include "AnalyzerSoftBlock.h"
#include <stdlib.h>

enum CyclicHistogramFilterMode
{
    None,
    Ideal,
    Nominal
};

class AnalyzerSoftCyclicHistogram final
{
public:
    AnalyzerSoftCyclicHistogram(const string &analyzer_schema);
    ~AnalyzerSoftCyclicHistogram();
    void Config(const json &config);
    static json CheckConfig(const json &config);
    void AddPoints(AnalyzerSoftBlock *inputBlock, int sampleRate);
    json GetCyclicHistogramForEvent(uint64_t eventStartTime);
    json GetCurrentCyclicHistogram();
    void WriteToDisk();
    void WriteToDatabase();
    void WriteToDatabase(uint64_t eventStartTime);
    void Clear();

private:
    bool pEnabled = false;
    CyclicHistogramFilterMode pVoltageFilterMode;
    uint32_t pMaxAmplitude;
    size_t pBinCount;
    Database *DB;
    bool skippedFirstPointSet;
    uint64_t startTime;
    uint64_t endTime;
    vector<vector<vector<uint64_t>>> cumulativeCount; // Variable used for storing cyclic histogram cumulative counts
    void ResizePhaseNumber(uint8_t numPhases);
    void ResizeCumulativeCount(uint8_t phaseNumber, size_t newSize);
};