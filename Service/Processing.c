/* ================================== zMeter ==================================
 * File:     Processing.c
 * Author:   Pedro Sanchez (3 de marzo de 2015)
 * Modified: Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#include <math.h>
#include "Main.h"

// Calculate RMS of each ½ Cycle
void CalculateCycleRMS(Work_t *work) {
  for(int i = 0; i < work->Cycles_Len; i++) {
    float Sum = 0;
    for(int j = work->Cycles[i].Start; j < work->Cycles[i].End; j++) Sum += pow(work->VoltageSamples[j], 2);
    work->Cycles[i].Voltage_RMS = sqrt(Sum / (work->Cycles[i].End - work->Cycles[i].Start));
  }
}
// Calculate voltage and current RMS values
void CalculateIntervalRMSs(Work_t *work, Params_t *params) {
  float V = 0.0;
  float I = 0.0;
  for(int i = 0; i < work->Samples; i++) {
    V += pow(work->VoltageSamples[i], 2.0);
    I += pow(work->CurrentSamples[i], 2.0);
  }
  params->RMS_V = sqrt(V / work->Samples);
  params->RMS_I = sqrt(I / work->Samples);
}

// Calculate frequency if nedded and return it, NaN otherwise
float CalculateFrequency(Work_t* work) {
  static uint64_t NumSamples = 0;    //Numero de muestras leidas desde el ultimo calculo
  static uint32_t NumCycles  = 0;    //Numero de ciclos desde ultimo calculo
  float  Frequency = NAN;
  NumSamples += work->Samples;
  NumCycles += work->Cycles_Len;
  if((((work->Timestamp / 1000000) % 10) == 0) && (NumSamples > (ADQUISITION_SAMPLEFREQ * 9))) {
    Frequency = NumCycles / (NumSamples / ADQUISITION_SAMPLEFREQ) / 2.0;
    NumSamples = 0;
    NumCycles = 0;  
  }
  return Frequency;
}
// Calculate active power
void CalculateActivePower(Work_t *work, Params_t *params) {
  float P = 0;
  for(int i = 0; i < work->Samples; i++) P += pow(work->VoltageSamples[i], 2.0) * pow(work->CurrentSamples[i], 2.0);
  params->Active_Power = sqrt(P / work->Samples);
}
// Calculate energy
void CalculateEnergy(Work_t* work, Params_t *params) {
  params->Energy = (params->Active_Power * work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
}
// Calculate voltage and current harmonics (return DC offset)
float CalculateHarmonicsRMS(Work_t *work, FFT_t *FFT_Voltage, FFT_t *FFT_Current, Params_t *params) {
  static const int FirstHarmonicBin = 10;
  static const int HarmonicHop      = 10;
  for(int i = 0; i < 50; i++) {
    params->VoltageArmonics[i] = sqrt(pow(FFT_Voltage->Result[i * HarmonicHop + FirstHarmonicBin].Real, 2.0) + pow(FFT_Voltage->Result[i * HarmonicHop + FirstHarmonicBin].Imag, 2.0));
    params->CurrentArmonics[i] = sqrt(pow(FFT_Current->Result[i * HarmonicHop + FirstHarmonicBin].Imag, 2.0) + pow(FFT_Current->Result[i * HarmonicHop + FirstHarmonicBin].Imag, 2.0));
  }
  return sqrt(pow(FFT_Voltage->Result[0].Real, 2.0) + pow(FFT_Voltage->Result[0].Imag, 2.0));
}

