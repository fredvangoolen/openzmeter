/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "CaptureInterface.h"
#include "Tools.h"
bool CaptureInterface::SaveWorkParams() {
  PrepareSentence("INSERT INTO agreg_10c(node, sampletime, rms_v, rms_i, active_power, apparent_power, power_factor, active_energy, fundamental_active_power, fundamental_reactive_power, fundamental_apparent_power, fundamental_power_factor, thd_v, thd_i, freq, phi, flaged, harmonics_v, harmonics_i) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  BindSingleUInt32(0);                                                           //node
  BindSingleUInt64(pWork.Timestamp);                                             //sampletime
  BindArrayFloat(pWork.RMS_V, (pTriphase == true) ? 3 : 1);                      //rms_v 
  BindArrayFloat(pWork.RMS_I, (pTriphase == true) ? 4 : 1);                      //rms_i
  BindArrayFloat(pWork.Active_Power, (pTriphase == true) ? 3 : 1);               //active_power
  BindArrayFloat(pWork.Apparent_Power, (pTriphase == true) ? 3 : 1);             //apparent_power
  BindArrayFloat(pWork.Power_Factor, (pTriphase == true) ? 3 : 1);               //power_factor
  BindArrayFloat(pWork.Active_Energy, (pTriphase == true) ? 3 : 1);              //active_energy
  BindArrayFloat(pWork.Fundamental_Active_Power, (pTriphase == true) ? 3 : 1);   //fundamental_active_power
  BindArrayFloat(pWork.Fundamental_Reactive_Power, (pTriphase == true) ? 3 : 1); //fundamental_reactive_power
  BindArrayFloat(pWork.Fundamental_Apparent_Power, (pTriphase == true) ? 3 : 1); //fundamental_apparent_power
  BindArrayFloat(pWork.Fundamental_Power_Factor, (pTriphase == true) ? 3 : 1);   //fundamental_power_factor
  BindArrayFloat(pWork.THD_V, (pTriphase == true) ? 3 : 1);                      //thd_v
  BindArrayFloat(pWork.THD_I, (pTriphase == true) ? 4 : 1);                      //thd_i
  BindSingleFloat(pWork.Frequency);                                              //frequency
  BindArrayFloat(pWork.Phi, (pTriphase == true) ? 3 : 1);                        //phi
  BindSingleBool(pWork.Flag);                                                    //flaged
  BindArrayFloat(pWork.VoltageHarmonics[0], (pTriphase == true) ? 150 : 50);     //harmonics_v
  BindArrayFloat(pWork.CurrentHarmonics[0], (pTriphase == true) ? 200 : 50);     //harmonics_i
  ExecSentence();
}
bool CaptureInterface::Analyze() {
  if(pTriphase == false) {
    FFT_t *FFT_Voltage = FFT_Start(pWork.VoltageSamples[0], pWork.Samples);         // STEP 1: Start calculating FFT (in parallel)
    FFT_t *FFT_Current = FFT_Start(pWork.CurrentSamples[0], pWork.Samples);
    //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
    float V = 0.0;
    float I = 0.0;
    float P = 0.0;
    for(int i = 0; i < pWork.Samples; i++) {
      V += pow(pWork.VoltageSamples[0][i], 2.0);
      I += pow(pWork.CurrentSamples[0][i], 2.0);
      P += pWork.VoltageSamples[0][i] * pWork.CurrentSamples[0][i];
    }
    pWork.Active_Power[0] = P / pWork.Samples;
    pWork.RMS_V[0] = sqrt(V / pWork.Samples);
    pWork.RMS_I[0] = sqrt(I / pWork.Samples);
    pWork.Apparent_Power[0] = pWork.RMS_V[0] * pWork.RMS_I[0];
//  Params->Flag = false; //Events_Detect(This->Work);
////  This->Params->Frequency = Analyzer_Frequency(Work);
    FFT_Wait(FFT_Voltage);
    for(int i = 0; i < 50; i++) {
//    Params->VoltageHarmonics[i] = sqrt(pow(FFT_Voltage->Result[10 + i * 10].Real, 2.0) + pow(FFT_Voltage->Result[10 + i * 10].Imag, 2.0)) / M_SQRT2 * 2.0;
//    if(i > 0) Params->THD_V += pow(Params->VoltageHarmonics[i], 2.0);
    }
//  Params->THD_V = sqrt(Params->THD_V) / Params->VoltageHarmonics[0];
  FFT_Wait(FFT_Current);
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
  free(FFT_Voltage);
  free(FFT_Current);
////  DB_Save(PARAMS_10C, 0, work->Timestamp, Params, true);
////  if((Work->SequenceID % 15) == 0) DB_Save(PARAMS_3S, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 60000) * 60000) != ((PrevTimestamp / 60000) * 60000)) DB_Save(PARAMS_1M, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 600000) * 600000) != ((PrevTimestamp / 600000) * 600000)) DB_Save(PARAMS_10M, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 3600000) * 3600000) != ((PrevTimestamp / 3600000) * 3600000)) DB_Save(PARAMS_1H, 0, PrevTimestamp, NULL, true);
//  PrevTimestamp = work->Timestamp;
//  free(work);
//  return Params;

    
    
    
    

  }


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
    pWork.RMS_V = (float*)malloc(sizeof(float*) * 3);
    pWork.RMS_I = (float*)malloc(sizeof(float*) * 4);
    pWork.Active_Power = (float*)malloc(sizeof(float*) * 3);
    pWork.Apparent_Power = (float*)malloc(sizeof(float*) * 3);
    pWork.Power_Factor = (float*)malloc(sizeof(float*) * 3);
    pWork.Fundamental_Active_Power = (float*)malloc(sizeof(float*) * 3);
    pWork.Fundamental_Apparent_Power = (float*)malloc(sizeof(float*) * 3);
    pWork.Fundamental_Reactive_Power = (float*)malloc(sizeof(float*) * 3);
    pWork.Fundamental_Power_Factor = (float*)malloc(sizeof(float*) * 3);
    pWork.Fundamental_Reactive_Energy = (float*)malloc(sizeof(float*) * 3);
    pWork.Active_Energy = (float*)malloc(sizeof(float*) * 3);
    pWork.THD_V = (float*)malloc(sizeof(float*) * 3);
    pWork.THD_I = (float*)malloc(sizeof(float*) * 3);
    pWork.VoltageHarmonics = (float**)malloc(sizeof(float*) * 3);
    for(int i = 0; i < 3; i++) pWork.VoltageHarmonics[i] = (float*)malloc(sizeof(float*) * 50);
    pWork.CurrentHarmonics = (float**)malloc(sizeof(float*) * 4);
    for(int i = 0; i < 3; i++) pWork.CurrentHarmonics[i] = (float*)malloc(sizeof(float*) * 50);
    pWork.Phi = (float*)malloc(sizeof(float*) * 3);
  } else {
    pWork.VoltageSamples = (float**)malloc(sizeof(float*) * 1);
    pWork.VoltageSamples[0] = (float*)malloc(sizeof(float*) * pMaxSamples);  
    pWork.CurrentSamples = (float**)malloc(sizeof(float*) * 1);
    pWork.CurrentSamples[0] = (float*)malloc(sizeof(float*) * pMaxSamples); 
    pWork.RMS_V = (float*)malloc(sizeof(float*) * 1);
    pWork.RMS_I = (float*)malloc(sizeof(float*) * 1);
    pWork.RMS_V = (float*)malloc(sizeof(float*) * 1);
    pWork.RMS_I = (float*)malloc(sizeof(float*) * 1);
    pWork.Active_Power = (float*)malloc(sizeof(float*) * 1);
    pWork.Apparent_Power = (float*)malloc(sizeof(float*) * 1);
    pWork.Power_Factor = (float*)malloc(sizeof(float*) * 1);
    pWork.Fundamental_Active_Power = (float*)malloc(sizeof(float*) * 1);
    pWork.Fundamental_Apparent_Power = (float*)malloc(sizeof(float*) * 1);
    pWork.Fundamental_Reactive_Power = (float*)malloc(sizeof(float*) * 1);
    pWork.Fundamental_Power_Factor = (float*)malloc(sizeof(float*) * 1);
    pWork.Fundamental_Reactive_Energy = (float*)malloc(sizeof(float*) * 1);
    pWork.Active_Energy = (float*)malloc(sizeof(float*) * 1);
    pWork.THD_V = (float*)malloc(sizeof(float*) * 1);
    pWork.THD_I = (float*)malloc(sizeof(float*) * 1);
    pWork.VoltageHarmonics = (float**)malloc(sizeof(float*) * 1);
    pWork.VoltageHarmonics[0] = (float*)malloc(sizeof(float*) * 50);
    pWork.CurrentHarmonics = (float**)malloc(sizeof(float*) * 4);
    pWork.CurrentHarmonics[0] = (float*)malloc(sizeof(float*) * 50);
    pWork.Phi = (float*)malloc(sizeof(float*) * 1);
  }
  ResetWork();
}
void CaptureInterface::ResetWork() {
  pWork.SequenceID = 0;
  pWork.Timestamp = 0;
  pWork.Samples = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) memset(pWork.VoltageSamples[i], 0, sizeof(float) * pMaxSamples);
  for(int i = 0; i < (pTriphase ? 4 : 1); i++) memset(pWork.CurrentSamples[i], 0, sizeof(float) * pMaxSamples);
  memset(pWork.Cycles, 0, sizeof(Cycle_t) * 20);
  pWork.Cycles_Len = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.RMS_V[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.RMS_I[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.Active_Power[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.Apparent_Power[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.Power_Factor[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.Fundamental_Active_Power[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.Fundamental_Apparent_Power[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.Fundamental_Reactive_Power[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.Fundamental_Power_Factor[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.Fundamental_Reactive_Energy[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.Active_Energy[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.THD_V[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.THD_I[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) memset(pWork.VoltageHarmonics[i], 0, sizeof(float) * 50);
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) memset(pWork.CurrentHarmonics[i], 0, sizeof(float) * 50);
  pWork.Frequency = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) pWork.Phi[i] = 0;
  pWork.Flag = false;
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