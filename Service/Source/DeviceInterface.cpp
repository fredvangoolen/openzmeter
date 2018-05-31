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
  pMaxCurrent      = 50;
  pMinCurrent      = 0.1;
  pTerminate       = false;
  pAnalyzers       = NULL;
  pAnalyzersCount  = 0;
  pActive          = false;
}
void DeviceInterface::AddAnalyzer(Analyzer *analyzer) {
  pAnalyzers = (Analyzer**)realloc(pAnalyzers, sizeof(Analyzer**) * (pAnalyzersCount) + 1);
  pAnalyzers[pAnalyzersCount] = analyzer;
  pAnalyzersCount++;
}
float DeviceInterface::GetSamplerate() {
  return pSamplerate;
}
float DeviceInterface::GetMaxCurrent() {
  return pMaxCurrent;
}
uint8_t DeviceInterface::GetAnalyzers() {
  return pAnalyzersCount;
}
Analyzer  *DeviceInterface::GetAnalyzers(uint8_t idx) {
  if(idx < pAnalyzersCount) return pAnalyzers[idx];
  return NULL;
}
bool DeviceInterface::BuffersAlloc(uint8_t voltageChannels, uint8_t currentChannels) {
  pMaxSamples = ceil(12.0 * pSamplerate / Devices::GetNominalFrequency());
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
  float Q = 1.0;
  float K = tan(M_PI * (Devices::GetNominalFrequency() / pSamplerate));
  float Norm = 1 / (1 + K / Q + K * K);
  pFilters.A0 = K / Q * Norm;
  pFilters.A1 = 0;
  pFilters.A2 = -pFilters.A0;
  pFilters.B1 = 2 * (K * K - 1) * Norm;
  pFilters.B2 = (1 - K / Q + K * K) * Norm;
  pFilters.Channels = (FilterChannel_t*)malloc(sizeof(FilterChannel_t) * pVoltageChannels);
  pFilters.Results  = (float*)malloc(sizeof(float) * pVoltageChannels);
  if((pFilters.Channels == NULL) || (pFilters.Results == NULL)) return false;
  for(int Chan = 0; Chan < pVoltageChannels; Chan++) pFilters.Channels[Chan].Stages = (FilterStage_t*)calloc(sizeof(FilterStage_t), DEVICEINTERFACE_FILTERSTAGES);
  return true;
}
void DeviceInterface::BuffersClear() {
  pSamples = 0;
}
float *DeviceInterface::BuffersAppend(float *voltageSamples, float *currentSamples) {
  for(int Chan = 0; Chan < pVoltageChannels; Chan++) {
    pVoltageSamples[Chan][pSamples] = voltageSamples[Chan];
    pFilters.Results[Chan] = voltageSamples[Chan];
    for(int Stage = 0; Stage < DEVICEINTERFACE_FILTERSTAGES; Stage++) {
      float Out = pFilters.Results[Chan] * pFilters.A0 + pFilters.Channels[Chan].Stages[Stage].Z1;
      pFilters.Channels[Chan].Stages[Stage].Z1 = pFilters.Results[Chan] * pFilters.A1 + pFilters.Channels[Chan].Stages[Stage].Z2 - pFilters.B1 * Out;
      pFilters.Channels[Chan].Stages[Stage].Z2 = pFilters.Results[Chan] * pFilters.A2 - pFilters.B2 * Out;
      pFilters.Results[Chan] = Out;
    }
  }
  for(int Chan = 0; Chan < pCurrentChannels; Chan++) pCurrentSamples[Chan][pSamples] = currentSamples[Chan];
  pSamples++;
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