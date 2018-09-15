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
#include "Analyzers/AnalyzerSoft.h"
// Register ============================================================================================================
bool DeviceVirtual::pClassRegistered = DeviceInterface::RegisterType("Virtual", DeviceVirtual::CreateInstance);
// Funciones ===========================================================================================================
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
    float Voltage[3];
    float Current[4];
    uint64_t Sampletime = Tools::GetTime();
    if((This->pAnalyzer->IsFinished() == true) && (((Sampletime - StartTime) * This->pSampleRate / 1000) > GeneratedSamples)) {
      Voltage[0] = This->pAnalyzer->GetNominalVoltage() * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pAnalyzer->GetNominalFrequency());
      Voltage[0] = Voltage[0] + Voltage[0] * 0.05 * sin((2.0 * M_PI * GeneratedSamples) / (This->pSampleRate * 300.0));
      Voltage[1] = This->pAnalyzer->GetNominalVoltage() * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pAnalyzer->GetNominalFrequency() - 2.0 * M_PI / 3.0);
      Voltage[1] = Voltage[1] + Voltage[1] * 0.05 * sin((2.0 * M_PI * (GeneratedSamples - This->pSampleRate * 100.0)) / (This->pSampleRate * 100.0));
      Voltage[2] = This->pAnalyzer->GetNominalVoltage() * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pAnalyzer->GetNominalFrequency() - 4.0 * M_PI / 3.0);
      Voltage[2] = Voltage[2] + Voltage[2] * 0.05 * sin((2.0 * M_PI * (GeneratedSamples - This->pSampleRate * 100.0)) / (This->pSampleRate * 100.0));
      Current[0] = 5.0 * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pAnalyzer->GetNominalFrequency());
      Current[0] = Current[0] + Current[0] * 0.1 * sin((2.0 * M_PI * GeneratedSamples) / (This->pSampleRate * 360.0));
      Current[1] = 5.0 * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pAnalyzer->GetNominalFrequency() - 2.0 * M_PI / 3.0);
      Current[1] = Current[1] + Current[1] * 0.1 * sin((2.0 * M_PI * (GeneratedSamples - This->pSampleRate * 120.0)) / (This->pSampleRate * 100.0));
      Current[2] = 5.0 * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pAnalyzer->GetNominalFrequency() - 4.0 * M_PI / 3.0);
      Current[2] = Current[2] + Current[2] * 0.1 * sin((2.0 * M_PI * (GeneratedSamples - This->pSampleRate * 240.0)) / (This->pSampleRate * 100.0));
      Current[3] =  Current[0] + Current[1] + Current[2];
      GeneratedSamples++;
      This->pAnalyzer->BufferAppend(Voltage, Current, Sampletime);
    } else {
      usleep(1000);
    }
  }
  return NULL;
}
DeviceInterface* DeviceVirtual::CreateInstance(json_object* config) {
  DeviceVirtual *This = new DeviceVirtual();
  Tools::LogMsg("INFO", "Starting DeviceVirtual driver...\n");
  float NominalVoltage   = 230.0;
  float NominalFrequency = 50.0;
  char *Name             = (char*)"Unnamed input";
  char *Serial           = (char*)"VIRTUAL";
  This->pAnalyzer        = NULL;
  This->pSampleRate      = 10000;
  json_object *Find;
  json_object_object_foreach(config, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Serial") == 0)) {
      Serial = (char*)json_object_get_string(JSON_Val);
    } else if((JSON_Type == json_type_double) && (strcmp(JSON_Key, "NominalVoltage") == 0)) {
      NominalVoltage = json_object_get_double(JSON_Val);
    } else if((JSON_Type == json_type_double) && (strcmp(JSON_Key, "NominalFrequency") == 0)) {
      NominalFrequency = json_object_get_double(JSON_Val);
    } else if((JSON_Type == json_type_double) && (strcmp(JSON_Key, "SampleRate") == 0)) {
      This->pSampleRate = json_object_get_double(JSON_Val);
    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "DisplayName") == 0)) {
      Name = (char*)json_object_get_string(JSON_Val);
    }
  }
  This->pAnalyzer = new AnalyzerSoft(This->pSampleRate, Serial, NominalVoltage, NominalFrequency, Name, 50.0, 0.1, AnalyzerInterface::PHASE3);
  if((json_object_object_get_ex(config, "Remote", &Find) == true) && (json_object_get_type(Find) == json_type_object)) This->pAnalyzer->ConfigRemote(Find);
  if(pthread_create(&This->pCaptureThread, NULL, Capture_Thread, This) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    delete This;
    return NULL;
  }
  return This;
}
DeviceVirtual::~DeviceVirtual() {
  pTerminate = true;
  Tools::LogMsg("INFO", "Waiting for adquisition thread ends...\n");
  pthread_join(pCaptureThread, NULL);
  if(pAnalyzer) delete pAnalyzer;
}
AnalyzerInterface *DeviceVirtual::FindAnalyzer(char *serial) {
  return (strcmp(serial, pAnalyzer->GetID()) == 0) ? pAnalyzer : NULL;
}
//uint32_t DeviceVirtual::SamplesCopy(float **dest, uint32_t offset, uint32_t len) {
//  for(uint i = 0; i < len; i++) {
//    uint32_t Pos = ((ReadPos + CAPTUREVIRTUAL_BUFFERSIZE) - offset + i) % CAPTUREVIRTUAL_BUFFERSIZE;
//    dest[i][0] = Buffer[Pos].VoltageA;
//    if(Triphase()) {
//      dest[i][1] = Buffer[Pos].VoltageB;
//      dest[i][2] = Buffer[Pos].VoltageC;
//    }
//  }
//  return len;
//}
//======================================================================================================================