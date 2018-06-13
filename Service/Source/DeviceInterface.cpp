/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <math.h>
#include "DeviceInterface.h"
#include "Capture.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
DeviceInterface::DeviceInterface(char *driver) {
  pDriverName      = driver;
  pVoltageSamples  = NULL;
  pCurrentSamples  = NULL;
  pVoltageChannels = 0;
  pCurrentChannels = 0;
  pSamplerate      = 20000;
  pMaxCurrent      = 50;
  pMinCurrent      = 0.1;
  pTerminate       = false;
  pAnalyzers       = NULL;
  pAnalyzersCount  = 0;
  pActive          = false;
  pReadPos         = 0;
  pWritePos        = 0;
  pUsed            = 0;
  pBuffLen         = 0;
  pLastUpdate      = 0;
}
void DeviceInterface::AddAnalyzer(Analyzer *analyzer) {
  pAnalyzers = (Analyzer**)realloc(pAnalyzers, sizeof(Analyzer*) * (pAnalyzersCount) + 1);
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
Analyzer *DeviceInterface::GetAnalyzers(uint8_t idx) {
  if(idx < pAnalyzersCount) return pAnalyzers[idx];
  return NULL;
}
bool DeviceInterface::BuffersAlloc(uint8_t voltageChannels, uint8_t currentChannels, float samplerate) {
  pBuffLen = DEVICEINTERFACE_BUFFERSECONDS * samplerate;
  pVoltageSamples = (float*)malloc(sizeof(float) * pBuffLen * voltageChannels);
  pCurrentSamples = (float*)malloc(sizeof(float) * pBuffLen * currentChannels);
  pVoltageChannels = voltageChannels;
  pCurrentChannels = currentChannels;
  pSamplerate      = samplerate;
  return (pVoltageSamples != 0) && (pCurrentSamples != 0);
}
void DeviceInterface::BuffersReset() {
  pUsed     = 0;
  pWritePos = 0;
  pReadPos  = 0;
}
bool DeviceInterface::BuffersAppend(float *voltageSamples, float *currentSamples) {
  if(pUsed == pBuffLen) return false;
  for(int8_t Channel = 0; Channel < pVoltageChannels; Channel++) pVoltageSamples[pWritePos * pVoltageChannels + Channel] = voltageSamples[Channel];
  for(int8_t Channel = 0; Channel < pCurrentChannels; Channel++) pCurrentSamples[pWritePos * pCurrentChannels + Channel] = currentSamples[Channel];
  pWritePos = (pWritePos + 1) % pBuffLen;
  pUsed++;
  uint64_t Sampletime = Tools::GetTime();
  if(pLastUpdate < (Sampletime - 500)) {
    char ThreadName[16];
    sprintf(ThreadName, "%s_%03d", pDriverName, pUsed * 100 / pBuffLen);
    pthread_setname_np(pCaptureThread, ThreadName);
    pLastUpdate = Sampletime;
  }
  for(uint8_t Analyzer = 0; Analyzer < pAnalyzersCount; Analyzer++) {
    if(pAnalyzers[Analyzer]->IsFinished() == false) return true;
  }
  bool Started = false;
  for(uint8_t Sample = 0; (Sample < 5) && (pUsed > 0) && (Started == false); Sample++) {
    for(uint8_t Analyzer = 0; Analyzer < pAnalyzersCount; Analyzer++) {
      if(pAnalyzers[Analyzer]->SampleAppend(&pVoltageSamples[pReadPos * pVoltageChannels], &pCurrentSamples[pReadPos * pCurrentChannels], Sampletime) == true) Started = true;
    }
    pReadPos = (pReadPos + 1) % pBuffLen;
    pUsed--;
  }
  return true;
}
  
  //  pVoltageFFT = (FFT**)malloc(sizeof(FFT*) * voltageChannels);
//  pCurrentFFT = (FFT**)malloc(sizeof(FFT*) * currentChannels);
//  for(int i = 0; i < voltageChannels; i++) {
//    char Name[16];
//    pVoltageSamples[i] = (float*)malloc(sizeof(float) * pBuffLen *);
//    sprintf(Name, "DeviceV2_FFT_V%d", i);
//    pVoltageFFT[i] = new FFT(pMaxSamples, Name);
//  }
//  for(int i = 0; i < currentChannels; i++) {
//    char Name[16];
//    pCurrentSamples[i] = (float*)malloc(sizeof(float) * pBuffLen);
//    sprintf(Name, "DeviceV2_FFT_C%d", i);
//    pCurrentFFT[i] = new FFT(pMaxSamples, Name);
//  }
  //  pSamples = 0;
//  float Q = 1.0;
//  float K = tan(M_PI * (Devices::GetNominalFrequency() / pSamplerate));
//  float Norm = 1 / (1 + K / Q + K * K);
//  pFilters.A0 = K / Q * Norm;
//  pFilters.A1 = 0;
//  pFilters.A2 = -pFilters.A0;
//  pFilters.B1 = 2 * (K * K - 1) * Norm;
//  pFilters.B2 = (1 - K / Q + K * K) * Norm;
//  pFilters.Channels = (FilterChannel_t*)malloc(sizeof(FilterChannel_t) * pVoltageChannels);
//  pFilters.Results  = (float*)malloc(sizeof(float) * pVoltageChannels);
//  if((pFilters.Channels == NULL) || (pFilters.Results == NULL)) return false;
//  for(int Chan = 0; Chan < pVoltageChannels; Chan++) pFilters.Channels[Chan].Stages = (FilterStage_t*)calloc(sizeof(FilterStage_t), DEVICEINTERFACE_FILTERSTAGES);
//  return true;
  
  //  if(pAnalyzerState == ANALYZER_PROCESSING) {
//    //Si algun analizador no ha finalizado
//    return  true;
//    This->pSamples = 0;
//      This->pCyclesCount = 0;
//      This->pCyclesLen[0] = 0;
//      This->pSampletime = Tools::GetTime() - (uint64_t)((This->pUsed * 1000) / (This->pSamplerate * This->pChannelCount));
//      This->pAnalyzerState = ANALYZER_WAIT;
//    }
//    if((This->pAnalyzerState == ANALYZER_WAIT) && (This->pUsed >= (This->pChannelCount * 10))) {
//      for(int i = 0; i < 10; i++) {
//        float VoltageChannels[3];
//        float CurrentChannels[4];
//        float *VoltagePos = VoltageChannels;
//        float *CurrentPos = CurrentChannels;
//        if(This->pChannelMask & 0x40) *VoltagePos++ = This->GetSample() * This->pVoltageGain[0];
//        if(This->pChannelMask & 0x20) *CurrentPos++ = This->GetSample() * This->pCurrentGain[0];
//        if(This->pChannelMask & 0x10) *VoltagePos++ = This->GetSample() * This->pVoltageGain[1];
//        if(This->pChannelMask & 0x08) *CurrentPos++ = This->GetSample() * This->pCurrentGain[1];
//        if(This->pChannelMask & 0x04) *VoltagePos++ = This->GetSample() * This->pVoltageGain[2];
//        if(This->pChannelMask & 0x02) *CurrentPos++ = This->GetSample() * This->pCurrentGain[2];
//        if(This->pChannelMask & 0x01) *CurrentPos++ = This->GetSample() * This->pCurrentGain[3];
//        float *FilterredValues = This->BuffersAppend(VoltageChannels, CurrentChannels);
//        //Si tengo los 20 pasos por 0 o he superado la longitud maxima lanzo los analyzadores y paso a estado ANALYZER_PROCESSING
//        if(This->pSamples == This->pMaxSamples) break;
//      }
//      if((This->pCyclesCount == 20) || (This->pSamples == This->pMaxSamples)) {
//        This->StartFFTs();
//        This->pAnalyzerState = ANALYZER_FFT;
//      }
//    }
//    if((This->pAnalyzerState == ANALYZER_FFT) && (This->IsFinishedFFTs() == true)) {
//      This->StartAnalyzers();
//      This->pAnalyzerState = ANALYZER_PROCESSING;
//    }
//  
//  
//  
//  
//  
//  
//  
//  for(int Chan = 0; Chan < pVoltageChannels; Chan++) {
//    pVoltageSamples[Chan][pSamples] = voltageSamples[Chan];
//    pFilters.Results[Chan] = voltageSamples[Chan];
//    for(int Stage = 0; Stage < DEVICEINTERFACE_FILTERSTAGES; Stage++) {
//      float Out = pFilters.Results[Chan] * pFilters.A0 + pFilters.Channels[Chan].Stages[Stage].Z1;
//      pFilters.Channels[Chan].Stages[Stage].Z1 = pFilters.Results[Chan] * pFilters.A1 + pFilters.Channels[Chan].Stages[Stage].Z2 - pFilters.B1 * Out;
//      pFilters.Channels[Chan].Stages[Stage].Z2 = pFilters.Results[Chan] * pFilters.A2 - pFilters.B2 * Out;
//      pFilters.Results[Chan] = Out;
//    }
//  }
//  for(int Chan = 0; Chan < pCurrentChannels; Chan++) pCurrentSamples[Chan][pSamples] = currentSamples[Chan];
//  pSamples++;

//void DeviceInterface::StartFFTs() {
//  for(int i = 0; i < pVoltageChannels; i++) pVoltageFFT[i]->Start(pVoltageSamples[i], pSamples);
//  for(int i = 0; i < pCurrentChannels; i++) pCurrentFFT[i]->Start(pCurrentSamples[i], pSamples);
//} 
//void DeviceInterface::StartAnalyzers() {
//  for(int i = 0; i < pAnalyzersCount; i++) pAnalyzers[i].Process->Start(&pAnalyzers[i].Data);
//}
//bool DeviceInterface::IsFinishedFFTs() {
//  for(int i = 0; i < pVoltageChannels; i++) {
//    if(pVoltageFFT[i]->IsFinished() == false) return false;
//  }
//  for(int i = 0; i < pCurrentChannels; i++) {
//    if(pCurrentFFT[i]->IsFinished() == false) return false;
//  }
//  return true;
//}
//bool DeviceInterface::IsFinishedAnalyzers() {
//  for(int i = 0; i < pAnalyzersCount; i++) {
//    if(pAnalyzers[i].Process->IsFinished() == false) return false;
//  }
//}
DeviceInterface::~DeviceInterface() {
  for(int i = 0; i < pAnalyzersCount; i++) delete pAnalyzers[i];
  free(pAnalyzers);
  if(pVoltageSamples) free(pVoltageSamples);
  if(pCurrentSamples) free(pCurrentSamples);
}
//uint32_t DeviceInterface::GetMaxSamples() {
//  return ceil((1.15 * pSamplerate * 10.0) / Devices::NominalFrequency);
//}

//float DeviceInterface::ApplyFilter(float in, Filter_t* filter) {
//  float out = in * filter->A0 + filter->Z1;
//  filter->Z1 = in * filter->A1 + filter->Z2 - filter->B1 * out;
//  filter->Z2 = in * filter->A2 - filter->B2 * out;
//  return out;
//}
//  pVoltageFFT      = NULL;
//  pCurrentFFT      = NULL;

  //  pMaxSamples      = 0;
