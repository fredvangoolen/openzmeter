#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include "Main.h"
// Tipos privados ======================================================================================================
static uint32_t        LastCycles[PROCESSING_FREQCYCLES];
static uint32_t        LastCyclesCount  = 0;
static uint32_t        LastCyclesPos    = 0;
static uint32_t        LastCyclesLenght = 0;
static pthread_mutex_t Mutex;                 //Mutex para bloquear acceso a muestras y resultados
static pthread_t       Thread;                //Descriptor del hilo de analisis
static bool            Terminate;             //Señal para detener el hilo de captura
static Work_t         *Work;                  //Current working samples
static Params_t       *Params;                //Current working params
// Funciones ===========================================================================================================
float CalcPhi(Complex_t voltage, Complex_t current) {
  float Tmp;
  if((voltage.Real == 0) && (voltage.Imag == 0)) 
    Tmp = 0;
  else if(voltage.Real < 0)
    Tmp = atan(voltage.Imag / voltage.Real);
  else
    Tmp = atan(voltage.Imag / voltage.Real) + M_PI;
  
  if((current.Real == 0) && (current.Imag == 0)) 
    return Tmp;
  else if(current.Real < 0)
    return Tmp - atan(current.Imag / current.Real);
  else
    return Tmp - atan(current.Imag / current.Real) + M_PI;
}
float Analyzer_Frequency(Work_t* work) {
  for(int i = 0; i < work->Cycles_Len; i++) {
    if(LastCyclesCount == PROCESSING_FREQCYCLES) LastCyclesLenght = LastCyclesLenght - LastCycles[LastCyclesPos];
    LastCycles[LastCyclesPos] = work->Cycles[i].End - work->Cycles[i].Start;
    LastCyclesLenght = LastCyclesLenght + LastCycles[LastCyclesPos];
    LastCyclesPos = (LastCyclesPos + 1) % PROCESSING_FREQCYCLES;
    if(LastCyclesCount < PROCESSING_FREQCYCLES) LastCyclesCount++;
  }
  return LastCyclesCount / (LastCyclesLenght / ADQUISITION_SAMPLEFREQ) / 2.0;
}
void *Analyzer_Thread(void *args) {
  uint64_t PrevTimestamp, Agreg3S_Start;
  pthread_setname_np(Thread, "Analysys");
  while(!Terminate) {
    Work = Adquisition_GetWork();
    if((Work == NULL) || (Work->SequenceID == 0)) {
      PrevTimestamp = Work->Timestamp;
      Agreg3S_Start = Work->Timestamp;
      continue;
    }
    Params = calloc(sizeof(Params_t), 1);
    FFT_t *FFT_Voltage = FFT_Start(Work->VoltageSamples, Work->Samples);         // STEP 1: Start calculating FFT (in parallel)
    FFT_t *FFT_Current = FFT_Start(Work->CurrentSamples, Work->Samples);
    //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
    float V = 0.0;
    float I = 0.0;
    float P = 0.0;
    for(int i = 0; i < Work->Samples; i++) {
      V += pow(Work->VoltageSamples[i], 2.0);
      I += pow(Work->CurrentSamples[i], 2.0);
      P += Work->VoltageSamples[i] * Work->CurrentSamples[i];
    }
    Params->Active_Power = P / Work->Samples;
    Params->RMS_V = sqrt(V / Work->Samples);
    Params->RMS_I = sqrt(I / Work->Samples);
    Params->Apparent_Power = Params->RMS_V * Params->RMS_I;
    Params->Flag = Events_Detect(Work);
    Params->Frequency = Analyzer_Frequency(Work);
    FFT_Wait(FFT_Voltage);
    for(int i = 0; i < 50; i++) {
      Params->VoltageHarmonics[i] = sqrt(pow(FFT_Voltage->Result[10 + i * 10].Real, 2.0) + pow(FFT_Voltage->Result[10 + i * 10].Imag, 2.0)) / M_SQRT2 * 2.0;
      if(i > 0) Params->THD_V += pow(Params->VoltageHarmonics[i], 2.0);
    }
    Params->THD_V = sqrt(Params->THD_V) / Params->VoltageHarmonics[0];
    FFT_Wait(FFT_Current);
    if(Params->RMS_I > PROCESSING_CURRENTOFFSET) {
      for(int i = 0; i < FFT_Voltage->Len; i++) {
        float CurrentHarmonic = sqrt(pow(FFT_Current->Result[i].Real, 2.0) + pow(FFT_Current->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
        float VoltageHarmonic = sqrt(pow(FFT_Voltage->Result[i].Real, 2.0) + pow(FFT_Voltage->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;  
        float PHI = CalcPhi(FFT_Voltage->Result[i], FFT_Current->Result[i]);
//        Params->Active_Power   += VoltageHarmonic * CurrentHarmonic * cos(PHI);
//        Params->Apparent_Power += VoltageHarmonic * CurrentHarmonic;
//        Params->Reactive_Power += VoltageHarmonic * CurrentHarmonic * sin(PHI);        
//        Params->Reactive_Power += VoltageHarmonic * CurrentHarmonic * sin(PHI);
        if(((i % 10) != 0) || (i > 500) || (i < 10)) continue;
        int Harmonic = (i / 10) - 1;
        Params->CurrentHarmonics[Harmonic] = CurrentHarmonic;
        if(Harmonic == 0) {
          Params->Fundamental_Active_Power   = VoltageHarmonic * CurrentHarmonic * cos(PHI);
          Params->Fundamental_Reactive_Power = VoltageHarmonic * CurrentHarmonic * sin(PHI);
          Params->Fundamental_Apparent_Power = VoltageHarmonic * CurrentHarmonic;
          Params->Fundamental_Power_Factor   = Params->Fundamental_Active_Power / Params->Fundamental_Apparent_Power;
          if(PHI > M_PI) PHI = -PHI + M_PI;  
          Params->Phi = PHI * 180.0 / M_PI;
        }
        if(Harmonic > 0) Params->THD_I += pow(CurrentHarmonic, 2.0);
      }
      Params->THD_I = sqrt(Params->THD_I) / Params->CurrentHarmonics[0];
      Params->Active_Energy = (Params->Active_Power * Work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
      Params->Power_Factor = Params->Active_Power / Params->Apparent_Power;
      Params->Fundamental_Reactive_Energy = (Params->Fundamental_Reactive_Power * Work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
    } else {
      Params->RMS_I = 0.0;
      memset(Work->CurrentSamples, 0, Work->Samples * sizeof(float));
      memset(FFT_Current->Result, 0, FFT_Current->Len * sizeof(Complex_t));
    }
    if(((Work->SequenceID) % 15 == 0) && (Work->Cycles_Len == 20)) {
      float FFT_V[600];
      float FFT_I[600];
      for(int i = 0; i < 600; i++) {
        FFT_V[i] = sqrt(pow(FFT_Voltage->Result[i].Real, 2.0) + pow(FFT_Voltage->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
        FFT_I[i] = sqrt(pow(FFT_Current->Result[i].Real, 2.0) + pow(FFT_Current->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
      }
      RequestServer_SetValues(Work->Timestamp, Params, Work->VoltageSamples, Work->CurrentSamples, FFT_V, FFT_I, Work->Samples, 600);
    }
    free(FFT_Voltage);
    free(FFT_Current);
    DB_Save(PARAMS_10C, 0, Work->Timestamp, Params, true);
    if((Work->SequenceID % 15) == 0) DB_Save(PARAMS_3S, 0, PrevTimestamp, NULL, true);
    if(((Work->Timestamp / 60000) * 60000) != ((PrevTimestamp / 60000) * 60000)) DB_Save(PARAMS_1M, 0, PrevTimestamp, NULL, true);
    if(((Work->Timestamp / 600000) * 600000) != ((PrevTimestamp / 600000) * 600000)) DB_Save(PARAMS_10M, 0, PrevTimestamp, NULL, true);
    if(((Work->Timestamp / 3600000) * 3600000) != ((PrevTimestamp / 3600000) * 3600000)) DB_Save(PARAMS_1H, 0, PrevTimestamp, NULL, true);
    PrevTimestamp = Work->Timestamp;
    free(Work);
  }
}
bool Analyzer_Start() {
  if(Adquisition_Start(ADQUISITION_DEVICE, ADQUISITION_BUFFERSIZE, 50.0) == false) return false;
  LogMsg("INFO", "Starting analyzer thread...\n");
  Terminate = false;
  Thread = 0;
  LastCyclesCount  = 0;
  LastCyclesPos    = 0;
  LastCyclesLenght = 0;
  if(pthread_mutex_init(&Mutex, NULL) != 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    Adquisition_Stop();
    return false;                                                                  
  } 
  if(pthread_create(&Thread, NULL, Analyzer_Thread, NULL) != 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    Adquisition_Stop();
    return false;
  }
  return true;
}
void Analyzer_Stop() {
  if(Thread != 0) {
    //  pthread_cond_signal(&WriteWait);
  //  pthread_cond_signal(&ReadWait);
    Terminate = true;
    LogMsg("INFO", "Waiting for analysis thread ends...\n");
    pthread_join(Thread, NULL);
    Adquisition_Stop();
  //  free(Buffer);
  }
}