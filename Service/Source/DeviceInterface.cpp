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
  pChannelMappings = NULL;
  pAnalyzersCount  = 0;
  pActive          = false;
  pReadPos         = 0;
  pWritePos        = 0;
  pUsed            = 0;
  pBuffLen         = 0;
  pLastUpdate      = 0;
}
void DeviceInterface::AddAnalyzer(AnalyzerSoft *analyzer) {
  AddAnalyzer(analyzer, 0, 0);
}
void DeviceInterface::AddAnalyzer(AnalyzerSoft *analyzer, uint8_t offsetV, uint8_t offsetC) {
  pAnalyzers = (AnalyzerSoft**)realloc(pAnalyzers, sizeof(AnalyzerSoft*) * (pAnalyzersCount) + 1);
  pChannelMappings = (ChannelMap_t*)realloc(pChannelMappings, sizeof(ChannelMap_t*) * (pAnalyzersCount) + 1);
  pAnalyzers[pAnalyzersCount] = analyzer;
  pChannelMappings[pAnalyzersCount].OffsetV = offsetV;
  pChannelMappings[pAnalyzersCount].OffsetC = offsetC;
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
AnalyzerSoft *DeviceInterface::GetAnalyzers(uint8_t idx) {
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
      if(pAnalyzers[Analyzer]->SampleAppend(&pVoltageSamples[pReadPos * pVoltageChannels + pChannelMappings[Analyzer].OffsetV], &pCurrentSamples[pReadPos * pCurrentChannels + pChannelMappings[Analyzer].OffsetC], Sampletime) == true) Started = true;
    }
    pReadPos = (pReadPos + 1) % pBuffLen;
    pUsed--;
  }
  return true;
}
DeviceInterface::~DeviceInterface() {
  for(int i = 0; i < pAnalyzersCount; i++) delete pAnalyzers[i];
  free(pAnalyzers);
  free(pChannelMappings);
  if(pVoltageSamples) free(pVoltageSamples);
  if(pCurrentSamples) free(pCurrentSamples);
}