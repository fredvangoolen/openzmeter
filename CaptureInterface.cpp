/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "CaptureInterface.h"
#include "Tools.h"
bool CaptureInterface::SaveWorkParams() {
  PrepareSentence("INSERT INTO agreg_10c(node, sampletime, rms_v, rms_i, active_power, apparent_power, power_factor, active_energy, fundamental_active_power, fundamental_reactive_power, fundamental_apparent_power, fundamental_power_factor, thd_v, thd_i, freq, phi, flaged, harmonics_v, harmonics_i) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  BindSingleUInt32(0);                                                                  //node
  BindSingleUInt64(pWork.Params.Timestamp);                                             //sampletime
  BindArrayFloat(pWork.Params.RMS_V, (pTriphase == true) ? 3 : 1);                      //rms_v 
  BindArrayFloat(pWork.Params.RMS_I, (pTriphase == true) ? 4 : 1);                      //rms_i
  BindArrayFloat(pWork.Params.Active_Power, (pTriphase == true) ? 3 : 1);               //active_power
  BindArrayFloat(pWork.Params.Apparent_Power, (pTriphase == true) ? 3 : 1);             //apparent_power
  BindArrayFloat(pWork.Params.Power_Factor, (pTriphase == true) ? 3 : 1);               //power_factor
  BindArrayFloat(pWork.Params.Active_Energy, (pTriphase == true) ? 3 : 1);              //active_energy
  BindArrayFloat(pWork.Params.Fundamental_Active_Power, (pTriphase == true) ? 3 : 1);   //fundamental_active_power
  BindArrayFloat(pWork.Params.Fundamental_Reactive_Power, (pTriphase == true) ? 3 : 1); //fundamental_reactive_power
  BindArrayFloat(pWork.Params.Fundamental_Apparent_Power, (pTriphase == true) ? 3 : 1); //fundamental_apparent_power
  BindArrayFloat(pWork.Params.Fundamental_Power_Factor, (pTriphase == true) ? 3 : 1);   //fundamental_power_factor
  BindArrayFloat(pWork.Params.THD_V, (pTriphase == true) ? 3 : 1);                      //thd_v
  BindArrayFloat(pWork.Params.THD_I, (pTriphase == true) ? 4 : 1);                      //thd_i
  BindSingleFloat(pWork.Params.Frequency);                                              //frequency
  BindArrayFloat(pWork.Params.Phi, (pTriphase == true) ? 3 : 1);                        //phi
  BindSingleBool(pWork.Params.Flag);                                                    //flaged
  BindArrayFloat(pWork.Params.VoltageHarmonics[0], (pTriphase == true) ? 150 : 50);     //harmonics_v
  BindArrayFloat(pWork.Params.CurrentHarmonics[0], (pTriphase == true) ? 200 : 50);     //harmonics_i
  ExecSentence();
  ResetWork();
}
void CaptureInterface::Analyze(float minimunCurrent) {
  FFT_t *FFT_Voltage[3];
  FFT_t *FFT_Current[4];
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) FFT_Voltage[i] = FFT_Start(pWork.VoltageSamples[i], pWork.Samples);
  for(int i = 0; i < (pTriphase ? 4 : 1); i++) FFT_Current[i] = FFT_Start(pWork.CurrentSamples[i], pWork.Samples);
  //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
  float V[3] = {0.0, 0.0, 0.0};
  float I[4] = {0.0, 0.0, 0.0, 0.0};
  float P[3] = {0.0, 0.0, 0.0};
  for(int i = 0; i < pWork.Samples; i++) {
    for(int j = 0; j < (pTriphase ? 3 : 1); j++) V[j] += pow(pWork.VoltageSamples[j][i], 2.0);
    for(int j = 0; j < (pTriphase ? 4 : 1); j++) I[j] += pow(pWork.CurrentSamples[j][i], 2.0);
    for(int j = 0; j < (pTriphase ? 3 : 1); j++) P[j] += pWork.VoltageSamples[j][i] * pWork.CurrentSamples[j][i];
  }
  for(int j = 0; j < (pTriphase ? 3 : 1); j++) pWork.Params.Active_Power[j] = P[j] / pWork.Samples;
  for(int j = 0; j < (pTriphase ? 3 : 1); j++) pWork.Params.RMS_V[j] = sqrt(V[j] / pWork.Samples);
  for(int j = 0; j < (pTriphase ? 4 : 1); j++) pWork.Params.RMS_I[j] = sqrt(I[j] / pWork.Samples);
  for(int j = 0; j < (pTriphase ? 3 : 1); j++) pWork.Params.Apparent_Power[j] = pWork.Params.RMS_V[j] * pWork.Params.RMS_I[j];
//  Params->Flag = false; //Events_Detect(This->Work);
  for(int i = 0; i < pWork.Cycles_Len; i++) {
    if(pLastCyclesCount == PROCESSING_FREQCYCLES) pLastCyclesLenght = pLastCyclesLenght - pLastCycles[pLastCyclesPos];
    pLastCycles[pLastCyclesPos] = pWork.Cycles[i].End - pWork.Cycles[i].Start;
    pLastCyclesLenght = pLastCyclesLenght + pLastCycles[pLastCyclesPos];
    pLastCyclesPos = (pLastCyclesPos + 1) % PROCESSING_FREQCYCLES;
    if(pLastCyclesCount < PROCESSING_FREQCYCLES) pLastCyclesCount++;
  }
  pWork.Params.Frequency = pLastCyclesCount / (pLastCyclesLenght / (float)pSampleRate) / 2.0;
  for(int j = 0; j < (pTriphase ? 3 : 1); j++) {
    FFT_Wait(FFT_Voltage[j]);
    for(int i = 0; i < 50; i++) {
      pWork.Params.VoltageHarmonics[j][i] = sqrt(pow(FFT_Voltage[j]->Result[10 + i * 10].Real, 2.0) + pow(FFT_Voltage[j]->Result[10 + i * 10].Imag, 2.0)) / M_SQRT2 * 2.0;
      if(i > 0) pWork.Params.THD_V[j] += pow(pWork.Params.VoltageHarmonics[j][i], 2.0);
    }
    pWork.Params.THD_V[j] = sqrt(pWork.Params.THD_V[j]) / pWork.Params.VoltageHarmonics[j][0];
  }
  for(int j = 0; j < (pTriphase ? 4 : 1); j++) {
    FFT_Wait(FFT_Current[j]);
    if(pWork.Params.RMS_I[j] < minimunCurrent) continue;
    for(int i = 10; i <= 500 ; i = i + 10) {
      int   Harmonic = (i / 10) - 1;
      float CurrentHarmonic = sqrt(pow(FFT_Current[j]->Result[i].Real, 2.0) + pow(FFT_Current[j]->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
      if(Harmonic > 0) pWork.Params.THD_I[j] += pow(CurrentHarmonic, 2.0);
      if(j < 3) {
        float VoltageHarmonic = pWork.Params.VoltageHarmonics[j][Harmonic];
        float PHI = CalcPhi(FFT_Voltage[j]->Result[i], FFT_Current[j]->Result[i]);
        pWork.Params.CurrentHarmonics[j][Harmonic] = CurrentHarmonic;
        if(Harmonic == 0) {
          pWork.Params.Fundamental_Active_Power[j]   = VoltageHarmonic * CurrentHarmonic * cos(PHI);
          pWork.Params.Fundamental_Reactive_Power[j] = VoltageHarmonic * CurrentHarmonic * sin(PHI);
          pWork.Params.Fundamental_Apparent_Power[j] = VoltageHarmonic * CurrentHarmonic;
          pWork.Params.Fundamental_Power_Factor[j]   = pWork.Params.Fundamental_Active_Power[j] / pWork.Params.Fundamental_Apparent_Power[j];
          PHI = fmod(PHI, 2.0 * M_PI);
          if(PHI > M_PI) PHI = PHI -  2.0 * M_PI;
          pWork.Params.Phi[j] = PHI * 180.0 / M_PI;
        }
      }
    }
    pWork.Params.THD_I[j] = sqrt(pWork.Params.THD_I[j]) / pWork.Params.CurrentHarmonics[j][0];
    if(j < 3) {
      pWork.Params.Active_Energy[j] = (pWork.Params.Active_Power[j] * pWork.Samples) / (pSampleRate * 3600.0);
      pWork.Params.Power_Factor[j] = pWork.Params.Active_Power[j] / pWork.Params.Apparent_Power[j];
      pWork.Params.Fundamental_Reactive_Energy[j] = (pWork.Params.Fundamental_Reactive_Power[j] * pWork.Samples) / (pSampleRate * 3600.0);
    }
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
  }
  for(int j = 0; j < (pTriphase ? 3 : 1); j++) free(FFT_Voltage[j]);
  for(int j = 0; j < (pTriphase ? 4 : 1); j++) free(FFT_Current[j]);
////  DB_Save(PARAMS_10C, 0, work->Timestamp, Params, true);
////  if((Work->SequenceID % 15) == 0) DB_Save(PARAMS_3S, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 60000) * 60000) != ((PrevTimestamp / 60000) * 60000)) DB_Save(PARAMS_1M, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 600000) * 600000) != ((PrevTimestamp / 600000) * 600000)) DB_Save(PARAMS_10M, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 3600000) * 3600000) != ((PrevTimestamp / 3600000) * 3600000)) DB_Save(PARAMS_1H, 0, PrevTimestamp, NULL, true);
//  PrevTimestamp = work->Timestamp;
//  free(work);
//  return Params;

    
    
    
    

  
}
//bool DB_Start(uint32_t *lastprice) {
//  struct timeval Time;
//  if(gettimeofday(&Time, NULL) != 0) {
//    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    return false;
//  }
//  LogMsg("INFO", "Starting data save thread...\n")INSERT INTO agreg_10c() VALUES(?,?,?,?);
//  if(pthread_mutex_init(&Mutex, NULL) != 0) {                                  
//    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strINSERT INTO agreg_10c() VALUES(?,?,?,?)error(errno));
//    return false;                                                                  
//  }
//  Buffer = (DBSave_t*)malloc(sizeof(DBSave_t) * Config.DB.QueueLen);
//  if(Buffer == NULL) {
//    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    pthread_mutex_destroy(&Mutex);
//    return false;
//  }
//  ReadPos = 0;
//  WritePos = 0;
//  Used = 0;
//  char ConnectionString[512];
//  sprintf(ConnectionString, "host=%s port=%d user=%s password=%s dbname=%s", Config.DB.Host, Config.DB.Port, Config.DB.User, Config.DB.Password, Config.DB.Database);
//  Database = PQconnectdb(ConnectionString);
//  if(PQstatus(Database) == CONNECTION_BAD) {
//    LogMsg("ERROR", "No se pudo abrir la base de datos (%s)\n", PQerrorMessage(Database));
//    pthread_mutex_destroy(&Mutex);
//    free(Buffer);
//    PQfinish(Database);
//    Database = NULL;
//    return false;
//  }
//  if(pthread_create(&Thread, NULL, DB_Thread, NULL) != 0) {
//    LogMsg("ERROR", "No se pudo abrir la base de datos (%s)\n", PQerrorMessage(Database));
//    pthread_mutex_destroy(&Mutex);
//    free(Buffer);
//    PQfinish(Database);
//    Database = NULL;
//    return false;
//  }
//  PGresult *Result = PQexec(Database, "SET synchronous_commit TO OFF");
//  if(PQresultStatus(Result) != PGRES_COMMAND_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
//  PQclear(Result);
//  char Tmp[128];
//  LogMsg("INFO", "Resuming from last execution...\n");
//  sprintf(Tmp, "SELECT STARTUP(%llu)", ((uint64_t)Time.tv_sec * 1000 + Time.tv_usec / 1000));
//  Result = PQexec(Database, Tmp);
//  if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
//  *lastprice = (PQgetisnull(Result, 0, 0)) ? (uint32_t)time(NULL) : atol(PQgetvalue(Result, 0, 0));
//  PQclear(Result);
//  return true;
//}
bool CaptureInterface::AllocParams(Params_t *params) {
  if(pTriphase == true) {
    params->RMS_V = (float*)malloc(sizeof(float*) * 3);
    params->RMS_I = (float*)malloc(sizeof(float*) * 4);
    params->Active_Power = (float*)malloc(sizeof(float*) * 3);
    params->Apparent_Power = (float*)malloc(sizeof(float*) * 3);
    params->Power_Factor = (float*)malloc(sizeof(float*) * 3);
    params->Fundamental_Active_Power = (float*)malloc(sizeof(float*) * 3);
    params->Fundamental_Apparent_Power = (float*)malloc(sizeof(float*) * 3);
    params->Fundamental_Reactive_Power = (float*)malloc(sizeof(float*) * 3);
    params->Fundamental_Power_Factor = (float*)malloc(sizeof(float*) * 3);
    params->Fundamental_Reactive_Energy = (float*)malloc(sizeof(float*) * 3);
    params->Active_Energy = (float*)malloc(sizeof(float*) * 3);
    params->THD_V = (float*)malloc(sizeof(float*) * 3);
    params->THD_I = (float*)malloc(sizeof(float*) * 3);
    params->VoltageHarmonics = (float**)malloc(sizeof(float*) * 3);
    for(int i = 0; i < 3; i++) params->VoltageHarmonics[i] = (float*)malloc(sizeof(float*) * 50);
    params->CurrentHarmonics = (float**)malloc(sizeof(float*) * 4);
    for(int i = 0; i < 3; i++) params->CurrentHarmonics[i] = (float*)malloc(sizeof(float*) * 50);
    params->Phi = (float*)malloc(sizeof(float*) * 3);
  } else {
    params->RMS_V = (float*)malloc(sizeof(float*) * 1);
    params->RMS_I = (float*)malloc(sizeof(float*) * 1);
    params->RMS_V = (float*)malloc(sizeof(float*) * 1);
    params->RMS_I = (float*)malloc(sizeof(float*) * 1);
    params->Active_Power = (float*)malloc(sizeof(float*) * 1);
    params->Apparent_Power = (float*)malloc(sizeof(float*) * 1);
    params->Power_Factor = (float*)malloc(sizeof(float*) * 1);
    params->Fundamental_Active_Power = (float*)malloc(sizeof(float*) * 1);
    params->Fundamental_Apparent_Power = (float*)malloc(sizeof(float*) * 1);
    params->Fundamental_Reactive_Power = (float*)malloc(sizeof(float*) * 1);
    params->Fundamental_Power_Factor = (float*)malloc(sizeof(float*) * 1);
    params->Fundamental_Reactive_Energy = (float*)malloc(sizeof(float*) * 1);
    params->Active_Energy = (float*)malloc(sizeof(float*) * 1);
    params->THD_V = (float*)malloc(sizeof(float*) * 1);
    params->THD_I = (float*)malloc(sizeof(float*) * 1);
    params->VoltageHarmonics = (float**)malloc(sizeof(float*) * 1);
    params->VoltageHarmonics[0] = (float*)malloc(sizeof(float*) * 50);
    params->CurrentHarmonics = (float**)malloc(sizeof(float*) * 4);
    params->CurrentHarmonics[0] = (float*)malloc(sizeof(float*) * 50);
    params->Phi = (float*)malloc(sizeof(float*) * 1);
  }
  return true;
}
CaptureInterface::CaptureInterface(uint32_t samplerate, bool triphase, enum Freq_t freq) {
  pSampleRate  = samplerate;
  pTriphase    = triphase;
  pNominalFreq = freq;
  pMaxSamples  = ceil(samplerate * 10.0 / (freq == FREQ_50 ? 42.5 : 51.0));
  if(triphase == true) {
    pWork.VoltageSamples = (float**)malloc(sizeof(float*) * 3);
    for(int i = 0; i < 3; i++) pWork.VoltageSamples[i] = (float*)malloc(sizeof(float*) * pMaxSamples);
    pWork.CurrentSamples = (float**)malloc(sizeof(float*) * 4);
    for(int i = 0; i < 4; i++) pWork.CurrentSamples[i] = (float*)malloc(sizeof(float*) * pMaxSamples);
  } else {
    pWork.VoltageSamples = (float**)malloc(sizeof(float*) * 1);
    pWork.VoltageSamples[0] = (float*)malloc(sizeof(float*) * pMaxSamples);  
    pWork.CurrentSamples = (float**)malloc(sizeof(float*) * 1);
    pWork.CurrentSamples[0] = (float*)malloc(sizeof(float*) * pMaxSamples); 
  }
  AllocParams(&pAgreg3S.Params);
  AllocParams(&pAgreg3S.Min);
  AllocParams(&pAgreg3S.Max);
  pAgreg3S.Samples = 0;
  AllocParams(&pAgreg1M.Params);
  AllocParams(&pAgreg1M.Min);
  AllocParams(&pAgreg1M.Max);
  pAgreg1M.Samples = 0;
  AllocParams(&pAgreg10M.Params);
  AllocParams(&pAgreg10M.Min);
  AllocParams(&pAgreg10M.Max);
  pAgreg10M.Samples = 0;
  AllocParams(&pAgreg1H.Params);
  AllocParams(&pAgreg1H.Min);
  AllocParams(&pAgreg1H.Max);
  pAgreg1H.Samples = 0;
  ResetWork();
  pLastCyclesCount  = 0;
  pLastCyclesPos    = 0;
  pLastCyclesLenght = 0;
  pWork.SequenceID  = 0;
}
void CaptureInterface::ResetParams(Params_t *params) {
  params->Timestamp = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->RMS_V[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->RMS_I[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Active_Power[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Apparent_Power[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Power_Factor[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Fundamental_Active_Power[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Fundamental_Apparent_Power[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Fundamental_Reactive_Power[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Fundamental_Power_Factor[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Fundamental_Reactive_Energy[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Active_Energy[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->THD_V[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->THD_I[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) memset(params->VoltageHarmonics[i], 0, sizeof(float) * 50);
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) memset(params->CurrentHarmonics[i], 0, sizeof(float) * 50);
  params->Frequency = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Phi[i] = 0;
  params->Flag = false;
}
void CaptureInterface::ResetWork() {
  pWork.SequenceID++;
  pWork.Samples = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) memset(pWork.VoltageSamples[i], 0, sizeof(float) * pMaxSamples);
  for(int i = 0; i < (pTriphase ? 4 : 1); i++) memset(pWork.CurrentSamples[i], 0, sizeof(float) * pMaxSamples);
  memset(pWork.Cycles, 0, sizeof(Cycle_t) * 20);
  pWork.Cycles_Len = 0;
  ResetParams(&pWork.Params);
}
CaptureInterface::~CaptureInterface() {
}
uint32_t CaptureInterface::GetMaxSamples() {
  return pMaxSamples;
}
uint32_t CaptureInterface::SampleRate() {
  return pSampleRate;
}
uint8_t CaptureInterface::NominalFrequency() {
  return (pNominalFreq == FREQ_50) ? 50 : 60;
}
bool CaptureInterface::Triphase() {
  return pTriphase;
}
float  CaptureInterface::CalcPhi(Complex_t voltage, Complex_t current) {
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
float CaptureInterface::ApplyFilter(float in, Filter_t* filter) {
  float out = in * filter->A0 + filter->Z1;
  filter->Z1 = in * filter->A1 + filter->Z2 - filter->B1 * out;
  filter->Z2 = in * filter->A2 - filter->B2 * out;
  return out;
}
void  *CaptureInterface::FFT_Thread(void *args) {
  FFT_t *FFT = (FFT_t *)args;
  fftwf_execute_dft_r2c(FFT->Plan, FFT->Input, (fftwf_complex*)FFT->Result);
  for(int i = 0; i < ceil(FFT->Len / 2 + 1); i++) {
    FFT->Result[i].Real = FFT->Result[i].Real / FFT->Len;
    FFT->Result[i].Imag = FFT->Result[i].Imag / FFT->Len;
  }
  FFT->Len = ceil(FFT->Len / 2 + 1);
  return NULL;  
}
void   CaptureInterface::FFT_Wait(FFT_t *fft) {
  if(pthread_join(fft->Thread, NULL) != 0) {  
    Tools::LogMsg("ERROR", "%s:%d -> (Unknown)\n", __func__, __LINE__);
    fftwf_destroy_plan(fft->Plan);
    return;
  }
  fftwf_destroy_plan(fft->Plan);
  return;
}
CaptureInterface::FFT_t *CaptureInterface::FFT_Start(float *data, uint32_t len) {
  FFT_t *FFT = (FFT_t *)malloc(sizeof(FFT_t) + sizeof(Complex_t) * ceil(len / 2));
  FFT->Plan = fftwf_plan_dft_r2c_1d(len, data, (fftwf_complex*)FFT->Result, FFTW_ESTIMATE);
  FFT->Len = len;
  FFT->Input = data;
  if(pthread_create(&FFT->Thread, NULL, FFT_Thread, (void*)FFT) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    fftwf_destroy_plan(FFT->Plan);
    free(FFT);
    return NULL;
  }
  return FFT;
}