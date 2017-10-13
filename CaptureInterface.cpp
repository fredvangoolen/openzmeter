/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "CaptureInterface.h"

bool CaptureInterface::SaveWorkParams() {



}
//bool DB_Start(uint32_t *lastprice) {
//  struct timeval Time;
//  if(gettimeofday(&Time, NULL) != 0) {
//    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    return false;
//  }
//  LogMsg("INFO", "Starting data save thread...\n");
//  if(pthread_mutex_init(&Mutex, NULL) != 0) {                                  
//    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
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
bool CaptureInterface::SaveWorkParams() {

}
CaptureInterface::~CaptureInterface() {
}
uint32_t CaptureInterface::GetMaxSamples() {
  return pMaxSamples;
}
uint32_t CaptureInterface::SampleRate() {
  return pSampleRate;
}
