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
    This->ResetWork();
    uint64_t Start = Tools::GetTime();
    This->pWork.Timestamp = Start;
    This->pWork.Cycles[0].Start = 0;
    for(int i = 0; ; i++) {
      This->pWork.VoltageSamples[0][i] = This->NominalVoltage * M_SQRT2 * sin(2.0 * M_PI * i / This->SampleRate() * This->NominalFrequency());
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
      }
    }
    This->pWork.SequenceID = This->SequenceID++;
    This->Analyze();
    uint64_t AnalyzeEnd = Tools::GetTime();
    This->SaveWorkParams();
    uint64_t SaveEnd = Tools::GetTime();
    usleep(200000 - (SaveEnd - Start));
    Tools::LogMsg("DEBUG", "Process of block %llu takes %.1fmS (%.1fmS mS with store)\n", This->pWork.SequenceID, (AnalyzeEnd - Start) / 1000.0, (SaveEnd - Start) / 1000.0);
  }
}
float CaptureVirtual::CalculateFrequency(Work_t* work) {
  static uint64_t NumSamples = 0;    //Numero de muestras leidas desde el ultimo calculo
  static uint32_t NumCycles  = 0;    //Numero de ciclos desde ultimo calculo
  float  Frequency = NAN;
  NumSamples += work->Samples;
  NumCycles += work->Cycles_Len;
  if((((work->Timestamp / 1000000) % 10) == 0) && (NumSamples > (SampleRate() * 9))) {
    Frequency = NumCycles / (NumSamples / SampleRate()) / 2.0;
    NumSamples = 0;
    NumCycles = 0;  
  }
  return Frequency;
}

//CaptureInterface::Params_t *BasicCapture::Analyze(Work_t *work) {
//  if((work == NULL) || (work->SequenceID == 0)) {
//    PrevTimestamp = work->Timestamp;
//    Agreg3S_Start = work->Timestamp;
//    return NULL;
//  }
//  Params_t *Params = (Params_t*)calloc(sizeof(Params_t), 1);
//  FFT_t *FFT_Voltage = FFT_Start(work->VoltageSamples, work->Samples);         // STEP 1: Start calculating FFT (in parallel)
//  FFT_t *FFT_Current = FFT_Start(work->CurrentSamples, work->Samples);
//  //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
//  float V = 0.0;
//  float I = 0.0;
//  float P = 0.0;
//  for(int i = 0; i < work->Samples; i++) {
//    V += pow(work->VoltageSamples[i], 2.0);
//    I += pow(work->CurrentSamples[i], 2.0);
//    P += work->VoltageSamples[i] * work->CurrentSamples[i];
//  }
//  Params->Active_Power = P / work->Samples;
//  Params->RMS_V = sqrt(V / work->Samples);
//  Params->RMS_I = sqrt(I / work->Samples);
//  Params->Apparent_Power = Params->RMS_V * Params->RMS_I;
//  Params->Flag = false; //Events_Detect(This->Work);
////  This->Params->Frequency = Analyzer_Frequency(Work);
//  FFT_Wait(FFT_Voltage);
//  for(int i = 0; i < 50; i++) {
//    Params->VoltageHarmonics[i] = sqrt(pow(FFT_Voltage->Result[10 + i * 10].Real, 2.0) + pow(FFT_Voltage->Result[10 + i * 10].Imag, 2.0)) / M_SQRT2 * 2.0;
//    if(i > 0) Params->THD_V += pow(Params->VoltageHarmonics[i], 2.0);
//  }
//  Params->THD_V = sqrt(Params->THD_V) / Params->VoltageHarmonics[0];
//  FFT_Wait(FFT_Current);
//  if(Params->RMS_I > PROCESSING_CURRENTOFFSET) {
//    for(int i = 10; i <= 500 ; i = i + 10) {
//      float CurrentHarmonic = sqrt(pow(FFT_Current->Result[i].Real, 2.0) + pow(FFT_Current->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
//      float VoltageHarmonic = sqrt(pow(FFT_Voltage->Result[i].Real, 2.0) + pow(FFT_Voltage->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;  
//      float PHI = CalcPhi(FFT_Voltage->Result[i], FFT_Current->Result[i]);
//      int Harmonic = (i / 10) - 1;
//      Params->CurrentHarmonics[Harmonic] = CurrentHarmonic;
//      if(Harmonic == 0) {
//        Params->Fundamental_Active_Power   = VoltageHarmonic * CurrentHarmonic * cos(PHI);
//        Params->Fundamental_Reactive_Power = VoltageHarmonic * CurrentHarmonic * sin(PHI);
//        Params->Fundamental_Apparent_Power = VoltageHarmonic * CurrentHarmonic;
//        Params->Fundamental_Power_Factor   = Params->Fundamental_Active_Power / Params->Fundamental_Apparent_Power;
//        PHI = fmod(PHI, 2.0 * M_PI);
//        if(PHI > M_PI) PHI = PHI -  2.0 * M_PI;
//        Params->Phi = PHI * 180.0 / M_PI;
//      }
//      if(Harmonic > 0) Params->THD_I += pow(CurrentHarmonic, 2.0);
//    }
//    Params->THD_I = sqrt(Params->THD_I) / Params->CurrentHarmonics[0];
//    Params->Active_Energy = (Params->Active_Power * work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
//    Params->Power_Factor = Params->Active_Power / Params->Apparent_Power;
//    Params->Fundamental_Reactive_Energy = (Params->Fundamental_Reactive_Power * work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
//  } else {
//    Params->RMS_I = 0.0;
//    memset(work->CurrentSamples, 0, work->Samples * sizeof(float));
//    memset(FFT_Current->Result, 0, FFT_Current->Len * sizeof(Complex_t));
//  }
//  if(((work->SequenceID) % 15 == 0) && (work->Cycles_Len == 20)) {
//    float FFT_V[600];
//    float FFT_I[600];
//    for(int i = 0; i < 600; i++) {
//      FFT_V[i] = sqrt(pow(FFT_Voltage->Result[i].Real, 2.0) + pow(FFT_Voltage->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
//      FFT_I[i] = sqrt(pow(FFT_Current->Result[i].Real, 2.0) + pow(FFT_Current->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
//    }
////    RequestServer_SetValues(work->Timestamp, Params, work->VoltageSamples, work->CurrentSamples, FFT_V, FFT_I, work->Samples, 600);
//  }
//  free(FFT_Voltage);
//  free(FFT_Current);
////  DB_Save(PARAMS_10C, 0, work->Timestamp, Params, true);
////  if((Work->SequenceID % 15) == 0) DB_Save(PARAMS_3S, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 60000) * 60000) != ((PrevTimestamp / 60000) * 60000)) DB_Save(PARAMS_1M, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 600000) * 600000) != ((PrevTimestamp / 600000) * 600000)) DB_Save(PARAMS_10M, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 3600000) * 3600000) != ((PrevTimestamp / 3600000) * 3600000)) DB_Save(PARAMS_1H, 0, PrevTimestamp, NULL, true);
//  PrevTimestamp = work->Timestamp;
//  free(work);
//  return Params;
//}
CaptureVirtual::CaptureVirtual(uint32_t samplerate, bool triphase, float nominalvoltage, float nominalcurrent, CaptureInterface::Freq_t baseFreq) : CaptureInterface(samplerate, triphase, baseFreq) {
  Tools::LogMsg("INFO", "Starting CaptureVirtual adquisition thread...\n");
  NominalVoltage   = nominalvoltage;
  NominalCurrent   = nominalcurrent;
  SequenceID       = 0;
  Terminate        = false;
  LastCyclesCount  = 0;
  LastCyclesPos    = 0;
  LastCyclesLenght = 0;
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