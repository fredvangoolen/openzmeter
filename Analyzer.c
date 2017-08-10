//#include <unistd.h>
#include <stdlib.h>
#include "types.h"
//#include "constants.h"
//#include "globalVars.h"
//#include "calculation_module.h"
//#include "utils.h"
//#include "debug.h"
//#include "thpool.h"

#include <string.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include "Main.h"

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

uint32_t LastCycles[PROCESSING_FREQCYCLES];
uint32_t LastCyclesCount  = 0;
uint32_t LastCyclesPos    = 0;
uint32_t LastCyclesLenght = 0;
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
Params_t *Analyzer_GetParams(Work_t *work) {
  Params_t *Params = calloc(sizeof(Params_t), 1);
  FFT_t *FFT_Voltage = FFT_Start(work->VoltageSamples, work->Samples);         // STEP 1: Start calculating FFT (in parallel)
  FFT_t *FFT_Current = FFT_Start(work->CurrentSamples, work->Samples);
  Params->Timestamp  = work->Timestamp;
  Params->SequenceID = work->SequenceID;
  //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
  float V = 0.0;
  float I = 0.0;
  float P = 0.0;
  for(int i = 0; i < work->Samples; i++) {
    V += pow(work->VoltageSamples[i], 2.0);
    I += pow(work->CurrentSamples[i], 2.0);
    P += work->VoltageSamples[i] * work->CurrentSamples[i];
  }
  Params->RMS_V = sqrt(V / work->Samples);
  Params->RMS_I = sqrt(I / work->Samples);
  Params->Flag = Events_Detect(work);
  Params->Frequency = Analyzer_Frequency(work);
  FFT_Wait(FFT_Voltage);
  //float THD_V = 0.0;
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
      Params->Active_Power   += VoltageHarmonic * CurrentHarmonic * cos(PHI);
      Params->Apparent_Power += VoltageHarmonic * CurrentHarmonic;
      Params->Reactive_Power += VoltageHarmonic * CurrentHarmonic * sin(PHI);
      if(((i % 10) != 0) || (i >= 500) || (i < 10)) continue;
      int Harmonic = (i / 10) - 1;
      Params->CurrentHarmonics[Harmonic] = CurrentHarmonic;
      if(Harmonic == 0) {
        Params->Fundamental_Active_Power   = VoltageHarmonic * CurrentHarmonic * cos(PHI);
        Params->Fundamental_Reactive_Power = VoltageHarmonic * CurrentHarmonic * sin(PHI);
        Params->Fundamental_Apparent_Power = VoltageHarmonic * CurrentHarmonic;
        Params->Fundamental_Power_Factor   = Params->Fundamental_Active_Power / Params->Fundamental_Apparent_Power;
        if(PHI > M_PI) PHI = -PHI + M_PI;        
        printf("PHI = %f\n", PHI * 180 / M_PI);

      }
      if(Harmonic > 0) Params->THD_I += pow(CurrentHarmonic, 2.0);
    }
    Params->THD_I = sqrt(Params->THD_I) / Params->CurrentHarmonics[0];

    //    for(int i = 0; i < 50; i++) {
//      Params->CurrentHarmonics[i] = sqrt(pow(FFT_Current->Result[10 + i * 10].Real, 2.0) + pow(FFT_Current->Result[10 + i * 10].Imag, 2.0)) / M_SQRT2 * 2.0;
//      float PHI = CalcPhi(FFT_Voltage->Result[10 + i * 10], FFT_Current->Result[10 + i * 10]);
//      if(i > 0) {
//      } else {
//        Params->Fundamental_Active_Power   = Params->VoltageHarmonics[i] * Params->CurrentHarmonics[i] * cos(PHI);
//        Params->Fundamental_Reactive_Power = Params->VoltageHarmonics[i] * Params->CurrentHarmonics[i] * sin(PHI);
//        Params->Fundamental_Apparent_Power = Params->VoltageHarmonics[i] * Params->CurrentHarmonics[i];
//        Params->Fundamental_Power_Factor   = Params->Fundamental_Active_Power / Params->Fundamental_Apparent_Power;
//      }
//              Active_Power += Params->VoltageHarmonics[i] * Params->CurrentHarmonics[i] * cos(PHI);
//        Apparent_Power += Params->VoltageHarmonics[i] * Params->CurrentHarmonics[i];
//        Reactive_Power += Params->VoltageHarmonics[i] * Params->CurrentHarmonics[i] * sin(PHI);
//    }

//    Params->Active_Power = P / work->Samples;
//    Params->Apparent_Power = Params->RMS_V * Params->RMS_I;
//    Params->Reactive_Power = sqrt(pow(Params->Apparent_Power, 2) - pow(Params->Active_Power, 2));

//    printf("ActivePower: %f (%f)\n", Params->Active_Power, 0);
//    printf("ReactivePower: %f (%f)\n", Params->Reactive_Power, 0);
//    printf("ApparentPower: %f (%f)\n", Params->Apparent_Power, 0);
    
    Params->Active_Energy = (Params->Active_Power * work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
    Params->Power_Factor = Params->Active_Power / Params->Apparent_Power;
    Params->Reactive_Energy = (Params->Reactive_Power * work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
  } else {
    Params->RMS_I = 0.0;
    memset(work->CurrentSamples, 0, work->Samples * sizeof(float));
    memset(FFT_Current->Result, 0, FFT_Current->Len * sizeof(Complex_t));
  }
    

      if(((work->SequenceID) % 15 == 0) && (work->Cycles_Len == 20)) {
    float FFT_V[600];
    float FFT_I[600];
    for(int i = 0; i < 600; i++) {
      FFT_V[i] = sqrt(pow(FFT_Voltage->Result[i].Real, 2.0) + pow(FFT_Voltage->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
      FFT_I[i] = sqrt(pow(FFT_Current->Result[i].Real, 2.0) + pow(FFT_Current->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
    }
    RequestServer_SetValues(Params, work->VoltageSamples, work->CurrentSamples, FFT_V, FFT_I, work->Samples, 600);
  }
Exit:
  free(FFT_Voltage);
  free(FFT_Current);
  free(work);
  return Params;
}