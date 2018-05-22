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
#include "CaptureVirtual.h"
#include "../Tools.h"
// Funciones ===========================================================================================================
float Variation(uint64_t step, float from, float to, uint64_t repeat) {
  uint64_t Pos = step % (repeat * 2);
  if(Pos < repeat) {
    return from + (to - from) * ((float)Pos / repeat);  
  } else {
    return from + (to - from) * (1.0 - ((float)(Pos - repeat) / repeat));  
  }
}
void  *CaptureVirtual::Capture_Thread(void *args) {
  int Prio = sched_get_priority_max(SCHED_RR);
  if(Prio == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    struct sched_param Param;
    Param.sched_priority = Prio - 20;
    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  }
  CaptureVirtual *This = (CaptureVirtual*)args;
  pthread_setname_np(This->CaptureThread, "CapVirt_Init");
  uint32_t Update = 0;
  uint64_t StartTime = Tools::GetTime();
  uint64_t GeneratedSamples = 0;
  while(!This->pTerminate) {
    pthread_mutex_lock(&This->CaptureMutex);
    while(This->Used > (CAPTUREVIRTUAL_BUFFERSIZE - CAPTUREVIRTUAL_SAMPLESREAD - This->SampleRate() * EVENT_ENVELOPETIME)) {
      if(This->pTerminate) return NULL;
      Tools::LogMsg("ERROR", "Capture buffer overflow! Posible lost of samples\n");
      pthread_cond_wait(&This->WriteWait, &This->CaptureMutex);
    }
    for(uint i = 0; i < CAPTUREVIRTUAL_SAMPLESREAD; i++) {
      float CurrentPhaseShift = Variation(GeneratedSamples, -0.2, 0.2, 30.0 * This->SampleRate());
      float CurrentA = Variation(GeneratedSamples, 1, 5, 30.0 * This->SampleRate());
      float CurrentB = Variation(GeneratedSamples, 1, 5, 40.0 * This->SampleRate());
      float CurrentC = Variation(GeneratedSamples, 1, 5, 50.0 * This->SampleRate());
      float VoltageA = Variation(GeneratedSamples, Tools::Electrical.NominalVoltage - 10, Tools::Electrical.NominalVoltage + 10, 300.0 * This->SampleRate());
      float VoltageB = Variation(GeneratedSamples, Tools::Electrical.NominalVoltage - 10, Tools::Electrical.NominalVoltage + 10, 310.0 * This->SampleRate());
      float VoltageC = Variation(GeneratedSamples, Tools::Electrical.NominalVoltage - 10, Tools::Electrical.NominalVoltage + 10, 320.0 * This->SampleRate());
      if(((GeneratedSamples / This->SampleRate()) >= 10) && ((GeneratedSamples / This->SampleRate()) < 20)) {
        VoltageB = Tools::Electrical.NominalVoltage * 0.001;
      }
      This->Buffer[This->WritePos].VoltageA = VoltageA * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->SampleRate() * Tools::Electrical.NominalFrequency);
      This->Buffer[This->WritePos].CurrentA = CurrentA * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->SampleRate() * Tools::Electrical.NominalFrequency + CurrentPhaseShift);
      if(This->Triphase()) {
        This->Buffer[This->WritePos].VoltageB = VoltageB * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->SampleRate() * Tools::Electrical.NominalFrequency - 2.0 * M_PI / 3.0);
        This->Buffer[This->WritePos].VoltageC = VoltageC * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->SampleRate() * Tools::Electrical.NominalFrequency - 4.0 * M_PI / 3.0);
        This->Buffer[This->WritePos].CurrentB = CurrentB * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->SampleRate() * Tools::Electrical.NominalFrequency - 2.0 * M_PI / 3.0 + CurrentPhaseShift);
        This->Buffer[This->WritePos].CurrentC = CurrentC * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->SampleRate() * Tools::Electrical.NominalFrequency - 4.0 * M_PI / 3.0 + CurrentPhaseShift);
        This->Buffer[This->WritePos].CurrentD = 0.0000000;
      }
      This->WritePos = (This->WritePos + 1) % CAPTUREVIRTUAL_BUFFERSIZE;
      GeneratedSamples++;
    }
    This->Used += CAPTUREVIRTUAL_SAMPLESREAD;
    pthread_cond_signal(&This->ReadWait);
    pthread_mutex_unlock(&This->CaptureMutex);
    while((StartTime + (GeneratedSamples / This->SampleRate()) * 1000.0) > Tools::GetTime()) usleep(1000);
    if((Update % 10) == 0) {
      char ThreadName[16];
      sprintf(ThreadName, "CapVirt_%03d", This->Used * 100 / CAPTUREVIRTUAL_BUFFERSIZE);
      pthread_setname_np(This->CaptureThread, ThreadName);
    }
    Update++;
  }
  return NULL;
}
void  *CaptureVirtual::Analize_Thread(void* args) {
  CaptureVirtual *This = (CaptureVirtual*)args;
  if(pthread_setschedprio(0, -10) == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  pthread_setname_np(This->AnalyzeThread, "AnalysisVirt");
  int Discard = 10;
  while(!This->pTerminate) {
    pthread_mutex_lock(&This->AnalyzeMutex);
    while(This->Used < (This->GetMaxSamples() + This->SampleRate() * EVENT_ENVELOPETIME)) {
      if(This->pTerminate) return NULL;
      pthread_cond_wait(&This->ReadWait, &This->AnalyzeMutex);
    }
    uint64_t Start = Tools::GetTime();
    This->pWork.Params.Timestamp = Start - ((This->Used * 1000) / CAPTUREVIRTUAL_SAMPLEFREQ);
    float LastSample = +20; //Siempre comienzo con cierto offset para que el primer flanco sea el positivo
    for(This->pWork.Samples = 0; This->pWork.Samples < This->GetMaxSamples(); This->pWork.Samples++) {
      This->pWork.VoltageSamples[0][This->pWork.Samples] = This->Buffer[This->ReadPos].VoltageA;
      This->pWork.CurrentSamples[0][This->pWork.Samples] = This->Buffer[This->ReadPos].CurrentA;
      if(This->Triphase()) {
        This->pWork.VoltageSamples[1][This->pWork.Samples] = This->Buffer[This->ReadPos].VoltageB;
        This->pWork.VoltageSamples[2][This->pWork.Samples] = This->Buffer[This->ReadPos].VoltageC;
        This->pWork.CurrentSamples[1][This->pWork.Samples] = This->Buffer[This->ReadPos].CurrentB;
        This->pWork.CurrentSamples[2][This->pWork.Samples] = This->Buffer[This->ReadPos].CurrentC;
        This->pWork.CurrentSamples[3][This->pWork.Samples] = This->Buffer[This->ReadPos].CurrentD;
      }
      float Out = This->pWork.VoltageSamples[0][This->pWork.Samples];
      for(int i = 0; i < 5; i++) Out = ApplyFilter(Out, &This->Filters[i]);
      if(((LastSample >= 0.0) && (Out < 0.0)) || ((LastSample <= 0.0) && (Out > 0.0))) {
        This->pWork.Cycles[This->pWork.Cycles_Len].End = This->pWork.Samples;
        This->pWork.Cycles_Len++;
        if(This->pWork.Cycles_Len == 20) break;
        This->pWork.Cycles[This->pWork.Cycles_Len].Start = This->pWork.Samples;
      }
      LastSample = Out;
      This->ReadPos = (This->ReadPos + 1) % CAPTUREVIRTUAL_BUFFERSIZE;
    }
    This->Used -= This->pWork.Samples;
    pthread_cond_signal(&This->WriteWait);
    pthread_mutex_unlock(&This->AnalyzeMutex);
    if(Discard > 0) {
      Discard--;
      This->ResetWork();
      This->ResetParams(&This->pWork.Params);
    } else {
      This->Analyze(0.1);
      uint64_t AnalyzeEnd = Tools::GetTime();
      This->SaveWorkParams();
      uint64_t SaveEnd = Tools::GetTime();
      Tools::LogMsg("DEBUG", "Process of block %" PRIu64 " takes %umS (%" PRIu64 "mS mS with store)\n", This->pWork.SequenceID, AnalyzeEnd - Start, SaveEnd - Start);
    }
  }
  return NULL;
}
CaptureVirtual::CaptureVirtual(bool triphase, float nominalcurrent, uint16_t id) : Analyzer(CAPTUREVIRTUAL_SAMPLEFREQ, triphase, id) {
  Tools::LogMsg("INFO", "Starting CaptureVirtual adquisition thread...\n");
  Buffer           = NULL;
  ReadPos          = 0;
  WritePos         = 0;
  Used             = 0;

  pthread_cond_init(&WriteWait, NULL);
  pthread_cond_init(&ReadWait, NULL);
  pthread_mutex_init(&AnalyzeMutex, NULL);
  pthread_mutex_init(&CaptureMutex, NULL);
  for(int i = 0; i < 5; i++) {
    float Q = 1.0;
    float K = tan(M_PI * ((float)Tools::Electrical.NominalFrequency / CAPTUREVIRTUAL_SAMPLEFREQ));
    float Norm = 1 / (1 + K / Q + K * K);
    Filters[i].A0 = K / Q * Norm;
    Filters[i].A1 = 0;
    Filters[i].A2 = -Filters[i].A0;
    Filters[i].B1 = 2 * (K * K - 1) * Norm;
    Filters[i].B2 = (1 - K / Q + K * K) * Norm;
    Filters[i].Z1 = 0.0;
    Filters[i].Z2 = 0.0;            
  }
  Buffer = (Sample_t*)malloc(CAPTUREVIRTUAL_BUFFERSIZE * sizeof(Sample_t));
  if(Buffer == NULL) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(NULL);
    return; 
  }
  if(pthread_create(&CaptureThread, NULL, Capture_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(NULL);
    free(Buffer);
    return;
  }
  if(pthread_create(&AnalyzeThread, NULL, Analize_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pTerminate = true;
    pthread_join(CaptureThread, NULL);
    free(Buffer);
    return;
  }
}
CaptureVirtual::~CaptureVirtual() {
  if(Buffer) {
    pthread_cond_signal(&WriteWait);
    pthread_cond_signal(&ReadWait);
    pTerminate = true;
    Tools::LogMsg("INFO", "Waiting for adquisition thread ends...\n");
    pthread_join(CaptureThread, NULL);
    pthread_join(AnalyzeThread, NULL);
    free(Buffer);
  }
}
uint32_t CaptureVirtual::SamplesCopy(float **dest, uint32_t offset, uint32_t len) {
  for(uint i = 0; i < len; i++) {
    uint32_t Pos = ((ReadPos + CAPTUREVIRTUAL_BUFFERSIZE) - offset + i) % CAPTUREVIRTUAL_BUFFERSIZE;
    dest[i][0] = Buffer[Pos].VoltageA;
    if(Triphase()) {
      dest[i][1] = Buffer[Pos].VoltageB;
      dest[i][2] = Buffer[Pos].VoltageC;
    }
  }
  return len;
}
//======================================================================================================================