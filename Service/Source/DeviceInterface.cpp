/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <math.h>
#include "DeviceInterface.h"
#include "Devices.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
//float DeviceInterface::ApplyFilter(float in, Filter_t* filter) {
//  float out = in * filter->A0 + filter->Z1;
//  filter->Z1 = in * filter->A1 + filter->Z2 - filter->B1 * out;
//  filter->Z2 = in * filter->A2 - filter->B2 * out;
//  return out;
//}
DeviceInterface::DeviceInterface() {
  pVoltageSamples  = NULL;
  pCurrentSamples  = NULL;
  pVoltageFFT      = NULL;
  pCurrentFFT      = NULL;
  pVoltageChannels = 0;
  pCurrentChannels = 0;
  pSamples         = 0;
  pMaxSamples      = 0;
  pSamplerate      = 20000;
  pTerminate       = false;
  Analyzers        = NULL;
  AnalyzersCount   = 0;
  Active           = false;
}
void DeviceInterface::AddAnalyzer(Analyzer *analyzer) {
  Analyzers = (Analyzer**)realloc(Analyzers, sizeof(Analyzer**) * (AnalyzersCount) + 1);
  Analyzers[AnalyzersCount] = analyzer;
  AnalyzersCount++;
}
bool DeviceInterface::BuffersAlloc(uint8_t voltageChannels, uint8_t currentChannels) {
  pMaxSamples = ceil(12.0 * pSamplerate / Devices::NominalFrequency);
  pVoltageSamples = (float**)malloc(sizeof(float*) * voltageChannels);
  pCurrentSamples = (float**)malloc(sizeof(float*) * currentChannels);
  pVoltageFFT = (FFT**)malloc(sizeof(FFT*) * voltageChannels);
  pCurrentFFT = (FFT**)malloc(sizeof(FFT*) * currentChannels);
  for(int i = 0; i < voltageChannels; i++) {
    pVoltageSamples[i] = (float*)malloc(sizeof(float) * pMaxSamples);
    pVoltageFFT[i] = new FFT(pMaxSamples);
  }
  for(int i = 0; i < currentChannels; i++) {
    pCurrentSamples[i] = (float*)malloc(sizeof(float) * pMaxSamples);
    pCurrentFFT[i] = new FFT(pMaxSamples);
  }
  pVoltageChannels = voltageChannels;
  pCurrentChannels = currentChannels;
  pSamples = 0;
}
void DeviceInterface::BuffersClear() {
  pSamples = 0;
}
float *DeviceInterface::BuffersAppend(float *voltageSamples, float *CurrentSamples) {
  return NULL;
}
void DeviceInterface::CalcFFTs() {
  for(int i = 0; i < pVoltageChannels; i++) pVoltageFFT[i]->Start(pVoltageSamples[i], pSamples);
  for(int i = 0; i < pCurrentChannels; i++) pCurrentFFT[i]->Start(pCurrentSamples[i], pSamples);
  for(int i = 0; i < pVoltageChannels; i++) pVoltageFFT[i]->Wait();
  for(int i = 0; i < pCurrentChannels; i++) pCurrentFFT[i]->Wait();
} 


DeviceInterface::~DeviceInterface() {
//  for(int i = 0; i < pAnalyzersCount; i++) delete pAnalyzers[i];
//  free(pAnalyzers);
}
//uint32_t DeviceInterface::GetMaxSamples() {
//  return ceil((1.15 * pSamplerate * 10.0) / Devices::NominalFrequency);
//}