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
void  *CaptureVirtual::Analize_Thread(void* args) {
  CaptureVirtual *This = (CaptureVirtual*)args;
  pthread_setname_np(This->AnalyzeThread, "AnalysisVirt");
  while(!This->Terminate) {
    uint64_t Start = Tools::GetTime();
    This->pWork.Params.Timestamp = Start;
    This->pWork.Cycles[0].Start = 0;
    for(int i = 0; ; i++) {
      This->pWork.VoltageSamples[0][i] = This->NominalVoltage * M_SQRT2 * sin(2.0 * M_PI * i / This->SampleRate() * This->NominalFrequency()) + rand() % 40;
      This->pWork.CurrentSamples[0][i] = This->NominalCurrent * M_SQRT2 * sin(2.0 * M_PI * i / This->SampleRate() * This->NominalFrequency());
      if(This->Triphase()) {
        This->pWork.VoltageSamples[1][i] = This->NominalVoltage * M_SQRT2 * sin(2.0 * M_PI * i / This->SampleRate() * This->NominalFrequency() + 1.0 * This->SampleRate() / This->NominalFrequency() / 3.0);
        This->pWork.VoltageSamples[2][i] = This->NominalVoltage * M_SQRT2 * sin(2.0 * M_PI * i / This->SampleRate() * This->NominalFrequency() + 2.0 * This->SampleRate() / This->NominalFrequency() / 3.0);
        This->pWork.CurrentSamples[1][i] = This->NominalCurrent * M_SQRT2 * sin(2.0 * M_PI * i / This->SampleRate() * This->NominalFrequency() + 1.0 * This->SampleRate() / This->NominalFrequency() / 3.0);
        This->pWork.CurrentSamples[2][i] = This->NominalCurrent * M_SQRT2 * sin(2.0 * M_PI * i / This->SampleRate() * This->NominalFrequency() + 2.0 * This->SampleRate() / This->NominalFrequency() / 3.0);
        This->pWork.CurrentSamples[3][i] = 0.0;
      }
      This->pWork.Samples++;
      if(i == (int)(This->SampleRate() / 2.0 / This->NominalFrequency() * (This->pWork.Cycles_Len + 1))) {
        This->pWork.Cycles[This->pWork.Cycles_Len].End = This->pWork.Samples;
        This->pWork.Cycles_Len++;
        if(This->pWork.Cycles_Len == 20) break;
        This->pWork.Cycles[This->pWork.Cycles_Len].Start = This->pWork.Samples;
      }
    }
    This->Analyze(0.1);
    uint64_t AnalyzeEnd = Tools::GetTime();
    This->SaveWorkParams();
    uint64_t SaveEnd = Tools::GetTime();
    if((SaveEnd - Start) <= 200) usleep((200 - (SaveEnd - Start)) * 1000);
    Tools::LogMsg("DEBUG", "Process of block %llu takes %llumS (%llumS with store)\n", This->pWork.SequenceID, AnalyzeEnd - Start, SaveEnd - Start);
  }
}
CaptureVirtual::CaptureVirtual(uint32_t samplerate, bool triphase, float nominalvoltage, float nominalcurrent, CaptureInterface::Freq_t baseFreq, uint16_t id) : CaptureInterface(samplerate, triphase, baseFreq, id) {
  Tools::LogMsg("INFO", "Starting CaptureVirtual adquisition thread...\n");
  NominalVoltage   = nominalvoltage;
  NominalCurrent   = nominalcurrent;
  Terminate        = false;
  if(pthread_create(&AnalyzeThread, NULL, Analize_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
}
CaptureVirtual::~CaptureVirtual() {
  Terminate = true;
  Tools::LogMsg("INFO", "Waiting for adquisition thread ends...\n");
  pthread_join(AnalyzeThread, NULL);
}
void CaptureVirtual::SamplesCopy(float *dest, int32_t offset, uint32_t len) {
 // for(int i = 0; i < len; i++) 
  //  dest[i] = Buffer[(ReadPos - offset + i) % Size].Voltage;
}
//======================================================================================================================