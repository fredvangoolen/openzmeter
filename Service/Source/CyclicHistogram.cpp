#include "CyclicHistogram.h"

CyclicHistogram::CyclicHistogram(const string &analyzer_schema, uint32_t nominalVoltage, size_t voltageBinSize)
{
    this->nominalVoltage = nominalVoltage;
    this->voltageBinSize = voltageBinSize;
    // Setup database structure
    DB = new Database(analyzer_schema);
    DB->ExecResource("SQL/Database_CyclicHistogram.sql");
    DB->ExecSentenceNoResult("DELETE FROM cychist");
    LOGFile::Debug("Cyclic Histogram database tables created\n");

    skippedFirstPointSet = false;
}

CyclicHistogram::~CyclicHistogram()
{
    // Cleanup memory
}

void CyclicHistogram::ResetCumulativeCount()
{
    // Reset all values in histogram to 0
    // std::fill(cumulativeCount->begin(), cumulativeCount->end(), 0);
}

void CyclicHistogram::AddPoints(uint8_t phaseNumber, float *pVoltageSamples, size_t arrSize, size_t offset)
{
    // Skip the first requested set of data points (tends to be unreliable data)
    if (!skippedFirstPointSet)
    {
        LOGFile::Warning("[CH] Skipping first set of data.\n");
        skippedFirstPointSet = true;
        return;
    }

    if(phaseNumber == 0){
        LOGFile::Error((to_string(pVoltageSamples[0]) + "\n").c_str());
    }

    uint8_t num_phases = phaseNumber + 1;

    // Check if vector is large enough for number of phases
    if (num_phases > cumulativeCount.size())
    {
        CyclicHistogram::ResizePhaseNumber(num_phases);
    }

    if ((arrSize + offset) > cumulativeCount[phaseNumber].size())
    {
        // Resize the cumulative count array to fit all data
        ResizeCumulativeCount(phaseNumber, arrSize + offset);
    }

    // Iterate through each point and find its appropriate bin index
    for (size_t i = 0; i < arrSize; i++)
    {
        float voltage = pVoltageSamples[i];
        if (abs(voltage) > nominalVoltage)
        {
            continue; // Skip this point
        }

        size_t sampleIndex = i + offset;

        // Convert voltage measurement into a discrete bin number (i.e. quantization)
        uint32_t binIndex = ((voltage + nominalVoltage) / (2 * nominalVoltage)) * (voltageBinSize - 1);

        // Perform the incremental count update
        cumulativeCount[phaseNumber][sampleIndex][binIndex] += 1;
    }
}

void CyclicHistogram::FlushToDisk()
{

    std::ofstream outputFile;
    outputFile.open("cyclichistogram.txt", std::ios::out);
    outputFile << "phase,sample,voltage_bin,count" << endl;

    for (size_t i = 0; i < cumulativeCount.size(); i++)
    {
        for (size_t j = 0; j < cumulativeCount[i].size(); j++)
        {
            for (size_t k = 0; k < cumulativeCount[i][j].size(); k++)
            {
                uint64_t value = cumulativeCount[i][j][k];
                if (value != 0)
                {
                    outputFile << i << "," << j << "," << k << "," << value << endl;
                }
            }
        }
    }

    outputFile.close();
}

void CyclicHistogram::ResizePhaseNumber(uint8_t numPhases)
{
    string message = "[CH] Resizing to accomodate " + to_string(numPhases) + " phases.\n";
    LOGFile::Warning(message.c_str());

    // Resize outer vector
    cumulativeCount.resize(numPhases);
}

void CyclicHistogram::ResizeCumulativeCount(uint8_t phaseNumber, size_t newSize)
{
    string message = "[CH] Resizing array from " + to_string(cumulativeCount[phaseNumber].size()) + " to " + to_string(newSize) + "\n";
    LOGFile::Warning(message.c_str());

    // Resize vector
    cumulativeCount[phaseNumber].resize(newSize, vector<uint64_t>(voltageBinSize));
}

vector<vector<vector<uint64_t>>> CyclicHistogram::GetCyclicHistogram()
{
    return cumulativeCount;
}