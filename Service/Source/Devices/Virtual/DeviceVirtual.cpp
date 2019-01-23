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
// Register ============================================================================================================
bool DeviceVirtual::pClassRegistered = DeviceVirtual::Init();
// Funciones ===========================================================================================================
bool DeviceVirtual::Init() {
  DeviceVirtual *This = new DeviceVirtual();
  This->pAnalyzer         = NULL;
  This->pSampleRate       = 10000;
  This->pVoltage   = 230.0;
  This->pFrequency = 50.0;
  This->pMutex            = PTHREAD_MUTEX_INITIALIZER;
  AddDevice(This);
  return true;
}
uint32_t DeviceVirtual::AnalyzersCount() {
  return (pAnalyzer == NULL) ? 0 : 1;
}
const char *DeviceVirtual::Serial() {
  return "VIRTUAL";
}
const char *DeviceVirtual::Driver() {
  return "Virtual";
}
const char *DeviceVirtual::Name() {
  return "Emulate most of real capture device for testing";
}
bool DeviceVirtual::ConfigDev(json_object *config) {
  if((pState == RUNNING) || (config == NULL) || (json_object_get_type(config) != json_type_object)) return false;
  Tools::LogMsg("INFO", "Configuring 'Virtual' driver...\n");
  bool AutoStart = false;
  json_object *Find;
  const char *Mode = NULL;
  if((json_object_object_get_ex(config, "Voltage", &Find) == true) && (json_object_get_type(Find) == json_type_double)) pVoltage = json_object_get_double(Find);
  if((json_object_object_get_ex(config, "Frequency", &Find) == true) && (json_object_get_type(Find) == json_type_double)) pFrequency = json_object_get_double(Find);
  if((json_object_object_get_ex(config, "SampleRate", &Find) == true) && (json_object_get_type(Find) == json_type_double)) pSampleRate = json_object_get_double(Find);
  if((json_object_object_get_ex(config, "AutoStart", &Find) == true) && (json_object_get_type(Find) == json_type_boolean)) AutoStart = json_object_get_boolean(Find);
  if((json_object_object_get_ex(config, "Mode", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Mode = (char*)json_object_get_string(Find);
  if((Mode == NULL) || (strcmp(Mode, "1PHASE") != 0) || (strcmp(Mode, "3PHASE") != 0) || (strcmp(Mode, "3PHASE+N") != 0)) Mode = "1PHASE";
  pMode = (strcmp(Mode, "1PHASE") == 0) ? AnalyzerInterface::PHASE1 : (strcmp(Mode, "3PHASE") == 0) ? AnalyzerInterface::PHASE3 : AnalyzerInterface::PHASE3N;
  if((pVoltage < 85) || (pVoltage > 300)) pVoltage = 230.0;
  if((pFrequency < 10) || (pFrequency > 80)) pFrequency = 50.0;
  if((pSampleRate < (pFrequency * 100))) pSampleRate = (pFrequency * 100);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    pConfig = config;
    if((AutoStart == true) && (Start() == false)) return false;
  }
  json_object_object_add(pConfig, "Voltage", json_object_new_double(pVoltage));
  json_object_object_add(pConfig, "Frequency", json_object_new_double(pFrequency));
  json_object_object_add(pConfig, "SampleRate", json_object_new_double(pSampleRate));
  json_object_object_add(pConfig, "Mode", json_object_new_string(Mode));
  json_object_object_add(pConfig, "AutoStart", json_object_new_boolean(AutoStart));
  if(Tools::SaveConfig() == false) return false;
  return true;
}
bool DeviceVirtual::Start() {
  pthread_mutex_lock(&pMutex);
  if(pState == RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  Tools::LogMsg("INFO", "Starting 'Virtual' driver...\n");
  pTerminate = false;
  char Name[128];
  snprintf(Name, sizeof(Name), "VIRTUAL_%s", (pMode == AnalyzerInterface::PHASE1) ? "1PHASE" : (pMode == AnalyzerInterface::PHASE3) ? "3PHASE" : "3PHASE+N");
  pAnalyzer = new AnalyzerSoft(Name, pMode, pSampleRate);
  if(pthread_create(&pCaptureThread, NULL, Capture_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
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
  Tools::LogMsg("INFO", "Stopping 'Virtual' driver...\n");
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
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    struct sched_param Param;
    Param.sched_priority = Prio - 20;
    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) Tools::LogMsg("WARN", "Cant raise thread priority -> %s\n", __func__, __LINE__, strerror(errno));
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
AnalyzerInterface *DeviceVirtual::FindAnalyzer(const char *serial) {
  if(pAnalyzer == NULL) return NULL;
  return (strcmp(serial, pAnalyzer->GetID()) == 0) ? pAnalyzer : NULL;
}
//======================================================================================================================
