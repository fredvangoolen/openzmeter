#pragma once

#include "Tools.h"
#include "LogFile.h"
#include "Database.h"
#include "AnalyzerSoftBlock.h"
#include <stdlib.h>
#include <vector>

enum CyclicHistogramVoltageFilterMode
{
    VOLTAGE_FILTER_NONE = 0,
    VOLTAGE_FILTER_NOMINAL = 1,
    VOLTAGE_FILTER_IDEAL = 2
};

enum CyclicHistogramCurrentFilterMode
{
    CURRENT_FILTER_NONE = 0,
    CURRENT_FILTER_NOMINAL = 1
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
    CyclicHistogramVoltageFilterMode pVoltageFilterMode;
    CyclicHistogramCurrentFilterMode pCurrentFilterMode;
    uint32_t pVoltageMaxAmplitude;
    uint32_t pCurrentMaxAmplitude;
    size_t pVoltageBinCount;
    size_t pCurrentBinCount;
    Database *DB;
    bool skippedFirstPointSet;
    uint64_t startTime;
    uint64_t endTime;
    bool nominalSamplesSaved;
    vector<vector<float>> nomVoltageSamples;           // Variable used for storing nominal samples (for filtering)
    vector<vector<float>> nomCurrentSamples;           // Variable used for storing nominal samples (for filtering)
    vector<vector<uint64_t>> nomVoltageSamplesCount;   // Used for averaging nominal samples
    vector<vector<uint64_t>> nomCurrentSamplesCount;   // Used for averaging nominal samples
    vector<vector<vector<uint64_t>>> voltageHistogram; // Variable used for storing cyclic histogram voltage cumulative counts
    vector<vector<vector<uint64_t>>> currentHistogram; // Variable used for storing cyclic histogram voltage cumulative counts
    void ResizeVoltageHistogram(uint8_t numVoltageChannels, size_t numSamples);
    void ResizeCurrentHistogram(uint8_t numCurrentChannels, size_t numSamples);
};