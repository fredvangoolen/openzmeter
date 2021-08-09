#pragma once

#include "Tools.h"
#include "LogFile.h"
#include "Database.h"
#include "AnalyzerSoftBlock.h"
#include <stdlib.h>
#include <vector>

enum CyclicHistogramFilterMode
{
    None = 0,
    Ideal = 1,
    Nominal = 2
};

class AnalyzerSoftCyclicHistogram final
{
public:
    AnalyzerSoftCyclicHistogram(const string &analyzer_schema);
    ~AnalyzerSoftCyclicHistogram();
    json GetConfig();
    void SetConfig(const json &config);
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
    bool nominalSamplesSaved;
    vector<vector<float>> nominalSamples;             // Variable used for storing nominal samples (for filtering)
    vector<vector<uint64_t>> nominalSamplesCount;          // Used for averaging nominal samples
    vector<vector<vector<uint64_t>>> cumulativeCount; // Variable used for storing cyclic histogram cumulative counts
    void ResizePhaseNumber(uint8_t numPhases);
    void ResizeCumulativeCount(uint8_t phaseNumber, size_t newSize);
};