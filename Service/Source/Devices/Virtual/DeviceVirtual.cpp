/* ================================== zMeter ===========================================================================
 * File:     CaptureVirtual.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <termios.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <libusb-1.0/libusb.h>
#include "DeviceVirtual.h"
#include "Tools.h"
// Funciones ===========================================================================================================
void DeviceVirtual::GetInstances(vector<Device*> &instances) {
  LOGFile::Info("Adding 'Virtual' device driver instance with serial 'VIRTUAL'\n");
  instances.push_back(new DeviceVirtual());
}
DeviceVirtual::DeviceVirtual() {
  pAnalyzer   = NULL;
  pSampleRate = 10000;
  pVoltage    = 230.0;
  pFrequency  = 50.0;
  pMode       = AnalyzerInterface::PHASE1;
  pSerial     = "VIRTUAL";
  pDriver     = "Virtual";
  pName       = "Emulate most of real capture device for testing";
  pMutex      = PTHREAD_MUTEX_INITIALIZER;
  Start();
}
uint32_t DeviceVirtual::AnalyzersCount() {
  return (pAnalyzer == NULL) ? 0 : 1;
}
bool DeviceVirtual::Start() {
  pthread_mutex_lock(&pMutex);
  if(pState == RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Loading 'Virtual' driver configuration\n");
  bool AutoStart = false;
  json Config = ConfigManager::ReadConfig("/Devices/" + pSerial);
  if(Config.is_object()) {
    if(Config["Voltage"].is_number()) pVoltage = Config["Voltage"];
    if(Config["Frequency"].is_number()) pFrequency = Config["Frequency"];
    if(Config["SampleRate"].is_number()) pSampleRate = Config["SampleRate"];
    if(Config["AutoStart"].is_boolean()) AutoStart = Config["AutoStart"];
    if(Config["Mode"].is_string() && ((Config["Mode"] == "1PHASE") || (Config["Mode"] == "3PHASE") || (Config["Mode"] == "3PHASE+N"))) pMode = (Config["Mode"] == "1PHASE") ? AnalyzerInterface::PHASE1 : (Config["Mode"] == "3PHASE") ? AnalyzerInterface::PHASE3 : AnalyzerInterface::PHASE3N;
  }
  if((pVoltage < 85) || (pVoltage > 300)) pVoltage = 230.0;
  if((pFrequency < 10) || (pFrequency > 80)) pFrequency = 50.0;
  if((pSampleRate < (pFrequency * 100))) pSampleRate = (pFrequency * 100);
  Config = json::object();
  Config["Voltage"] = pVoltage;
  Config["Frequency"] = pFrequency;
  Config["SampleRate"] = pSampleRate;
  Config["Mode"] = (pMode == AnalyzerInterface::PHASE1) ? "1PHASE" : (pMode == AnalyzerInterface::PHASE3) ? "3PHASE" : "3PHASE+N";
  Config["AutoStart"] = AutoStart;
  ConfigManager::WriteConfig("/Devices/" + pSerial, Config);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    if(AutoStart == false) {
      pthread_mutex_unlock(&pMutex);
      return true;
    }
  }
  LOGFile::Info("Starting 'Virtual' driver\n");
  pTerminate = false;
  pAnalyzer = new AnalyzerSoft(pSerial + "_" + ((pMode == AnalyzerInterface::PHASE1) ? "1PHASE" : (pMode == AnalyzerInterface::PHASE3) ? "3PHASE" : "3PHASE+N"), pMode, pSampleRate);
  if(pthread_create(&pCaptureThread, NULL, Capture_Thread, this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    delete pAnalyzer;
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pState = RUNNING;
  pthread_mutex_unlock(&pMutex);
  return true;
}
bool DeviceVirtual::Stop() {
  pthread_mutex_lock(&pMutex);
  if(pState != RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Stopping 'Virtual' driver...\n");
  pTerminate = true;
  pthread_join(pCaptureThread, NULL);
  if(pAnalyzer) delete pAnalyzer;
  pAnalyzer = NULL;
  pState = STOPPED;  
  pthread_mutex_unlock(&pMutex);
  return true;
}
void  *DeviceVirtual::Capture_Thread(void *args) {
  int Prio = sched_get_priority_max(SCHED_RR);
  if(Prio == -1) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    struct sched_param Param;
    Param.sched_priority = Prio - 20;
    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) LOGFile::Warning("Cant raise thread priority -> %s\n", __func__, __LINE__, strerror(errno));
  }
  DeviceVirtual *This = (DeviceVirtual*)args;
  pthread_setname_np(This->pCaptureThread, "DeviceVirtual");
  uint64_t StartTime        = Tools::GetTime();
  uint64_t GeneratedSamples = 0;
  while(!This->pTerminate) {
    vector<float> Voltage(3);
    vector<float> Current(4);
    uint64_t Sampletime = Tools::GetTime();
    if((This->pAnalyzer->IsFinished() == true) && (((Sampletime - StartTime) * This->pSampleRate / 1000) > GeneratedSamples)) {
      Voltage[0] = This->pAnalyzer->GetNominalVoltage() * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency);
      Voltage[0] = Voltage[0] + Voltage[0] * 0.05 * sin((2.0 * M_PI * GeneratedSamples) / (This->pSampleRate * 300.0));
      Voltage[1] = This->pAnalyzer->GetNominalVoltage() * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 2.0 * M_PI / 3.0);
      Voltage[1] = Voltage[1] + Voltage[1] * 0.05 * sin((2.0 * M_PI * (GeneratedSamples - This->pSampleRate * 100.0)) / (This->pSampleRate * 100.0));
      Voltage[2] = This->pAnalyzer->GetNominalVoltage() * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 4.0 * M_PI / 3.0);
      Voltage[2] = Voltage[2] + Voltage[2] * 0.05 * sin((2.0 * M_PI * (GeneratedSamples - This->pSampleRate * 100.0)) / (This->pSampleRate * 100.0));
      Current[0] = -5.0 * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency);
      Current[0] = Current[0] + Current[0] * 0.1 * sin((2.0 * M_PI * GeneratedSamples) / (This->pSampleRate * 360.0));
      Current[1] = -5.0 * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 2.0 * M_PI / 3.0);
      Current[1] = Current[1] + Current[1] * 0.1 * sin((2.0 * M_PI * (GeneratedSamples - This->pSampleRate * 120.0)) / (This->pSampleRate * 100.0));
      Current[2] = -5.0 * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 4.0 * M_PI / 3.0);
      Current[2] = Current[2] + Current[2] * 0.1 * sin((2.0 * M_PI * (GeneratedSamples - This->pSampleRate * 240.0)) / (This->pSampleRate * 100.0));
      Current[3] = Current[0] + Current[1] + Current[2];
      GeneratedSamples++;
      This->pAnalyzer->BufferAppend(Voltage, Current, Sampletime);
    } else {
      usleep(1000);
    }
  }
  return NULL;
}
DeviceVirtual::~DeviceVirtual() {
  Stop();
}
AnalyzerInterface *DeviceVirtual::FindAnalyzer(string serial) {
  if(pAnalyzer == NULL) return NULL;
  return (serial == pAnalyzer->Serial()) ? pAnalyzer : NULL;
}
//======================================================================================================================
