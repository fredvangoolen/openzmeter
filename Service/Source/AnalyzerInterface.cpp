/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.cpp
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/ 
#include "AnalyzerInterface.h"
//----------------------------------------------------------------------------------------------------------------------
AnalyzerInterface::AnalyzerInterface(char *id, float nominal_voltage, float nominal_frequency, char *name, float max_current, float min_current, AnalyzerMode mode) : Database(NULL) {
  pID = id;
  pNominalVoltage = nominal_voltage;
  pNominalFrequency = nominal_frequency;
  pName = name;
  pMaxCurrent = max_current;
  pMinCurrent = min_current;
  pMode = mode;
  PrepareSentence("CREATE SCHEMA IF NOT EXISTS ?");
  BindSingleString(id, true);
  ExecSentence();
  PrepareSentence("CREATE TABLE IF NOT EXISTS ?.aggreg_3s ( "
                  "  sampletime bigint NOT NULL,"
                  "  rms_v real[],"
                  "  rms_i real[],"
                  "  active_power real[],"
                  "  active_energy real[],"
                  "  reactive_power real[],"
                  "  freq real,"
                  "  flag boolean,"
                  "  reactive_energy real[],"
                  "CONSTRAINT aggreg_3s_pkey PRIMARY KEY (sampletime)"
                  ") WITH (OIDS=FALSE)");
  BindSingleString(id, true);
  ExecSentence();
  
//CREATE TABLE IF NOT EXISTS aggreg_1m (
//  node smallint NOT NULL,
//  sampletime bigint NOT NULL,
//  rms_v real[],
//  rms_i real[],
//  active_power real[],
//  active_energy real[],
//  fundamental_reactive_power real[],
//  freq real,
//  flag boolean,
//  fundamental_reactive_energy real[],
//  CONSTRAINT aggreg_1m_pkey PRIMARY KEY (sampletime, node)
//) WITH (OIDS=FALSE);
//ALTER TABLE aggreg_1m OWNER TO openzmeter;
//
//CREATE TABLE IF NOT EXISTS aggreg_ext_1m (
//  node_ext smallint NOT NULL,
//  sampletime_ext bigint NOT NULL,
//  power_factor real[],
//  thd_v real[],
//  thd_i real[],
//  phi real[],
//  harmonics_v real[],
//  harmonics_i real[],
//  CONSTRAINT aggreg_ext_1m_pkey PRIMARY KEY (sampletime_ext, node_ext)
//) WITH (OIDS=FALSE);
//ALTER TABLE aggreg_ext_1m OWNER TO openzmeter;
//
//CREATE TABLE IF NOT EXISTS aggreg_10m (
//  node smallint NOT NULL,
//  sampletime bigint NOT NULL,
//  rms_v real[],
//  rms_i real[],
//  active_power real[],
//  active_energy real[],
//  fundamental_reactive_power real[],
//  freq real,
//  flag boolean,
//  fundamental_reactive_energy real[],
//  CONSTRAINT aggreg_10m_pkey PRIMARY KEY (sampletime, node)
//) WITH (OIDS=FALSE);
//ALTER TABLE aggreg_10m OWNER TO openzmeter;
//
//CREATE TABLE IF NOT EXISTS aggreg_1h (
//  node smallint NOT NULL,
//  sampletime bigint NOT NULL,
//  rms_v real[],
//  rms_i real[],
//  active_power real[],
//  active_energy real[],
//  fundamental_reactive_power real[],
//  freq real,
//  flag boolean,
//  fundamental_reactive_energy real[],
//  CONSTRAINT aggreg_1h_pkey PRIMARY KEY (sampletime, node)
//) WITH (OIDS=FALSE);
//ALTER TABLE aggreg_1h OWNER TO openzmeter;
//
//CREATE TABLE IF NOT EXISTS aggreg_stat_1h (
//  node_ext smallint NOT NULL,
//  sampletime_ext bigint NOT NULL,
//  stat_voltage smallint[],
//  stat_frequency smallint[],
//  stat_thd smallint[],
//  stat_harmonics smallint[],
//  CONSTRAINT aggreg_stat_1h_pkey PRIMARY KEY (sampletime_ext, node_ext)
//) WITH (OIDS=FALSE);
//ALTER TABLE aggreg_stat_1h OWNER TO openzmeter;
//
//CREATE TABLE IF NOT EXISTS events (
//  node smallint NOT NULL,  
//  starttime bigint NOT NULL,
//  duration  bigint,
//  type character varying(16),
//  voltage real,
//  percent real,
//  samples real[],
//  CONSTRAINT events_pkey PRIMARY KEY (starttime, node)
//) WITH (OIDS=FALSE);
//ALTER TABLE events OWNER TO openzmeter;
//  
//  
  
  
  
  
  
  PrepareSentence("INSERT INTO devices(serial, nominal_voltage, nominal_frequency, name, max_current, min_current, mode) "
                  "VALUES(?, ?, ?, ?, ?, ?, ?) "
                  "ON CONFLICT(serial) DO UPDATE SET nominal_voltage = excluded.nominal_voltage, nominal_frequency = excluded.nominal_frequency, name = excluded.name, max_current = excluded.max_current, min_current = excluded.mincurrent, mode = excluded.mode");
  BindSingleString(id);
  BindSingleFloat(nominal_voltage);
  BindSingleFloat(nominal_frequency);
  BindSingleString(name);
  BindSingleFloat(max_current);
  BindSingleFloat(min_current);
  BindSingleString((mode == PHASE1) ? "1PHASE" : (mode == PHASE3) ? "3PHASE" : "3PHASE+N");
}
char *AnalyzerInterface::GetID() {
  return pID;
}
json_object *AnalyzerInterface::GetJSONDescriptor() {
  json_object *Device = json_object_new_object();
  json_object_object_add(Device, "ID", json_object_new_string(pID));
  json_object_object_add(Device, "NominalVoltage", json_object_new_double(pNominalVoltage));
  json_object_object_add(Device, "NominalFreq", json_object_new_double(pNominalFrequency));
  json_object_object_add(Device, "Name", json_object_new_string(pName));
  json_object_object_add(Device, "MaxCurrent", json_object_new_double(pMaxCurrent));
  json_object_object_add(Device, "MinCurrent", json_object_new_double(pMinCurrent));
  json_object_object_add(Device, "Mode", json_object_new_string((pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N"));
  return Device;
}
//float             AnalyzerInterface::pNominalVoltage        = 230.0;
//float             AnalyzerInterface::pNominalFrequency      = 50.0;

  //  struct json_object *Find;
//  if((json_object_object_get_ex(config, "NominalVoltage", &Find) == true) && (json_object_get_type(Find) == json_type_double)) {
//    pNominalVoltage = json_object_get_double(Find);
//  }
//  if((json_object_object_get_ex(config, "NominalFrequency", &Find) == true) && (json_object_get_type(Find) == json_type_double)) {
//    pNominalFrequency = json_object_get_double(Find);
//  }
//  if((json_object_object_get_ex(config, "Devices", &Find) == true) && (json_object_get_type(Find) == json_type_array)) {

//  pDriverName      = driver;
//  pVoltageSamples  = NULL;
//  pCurrentSamples  = NULL;
//  pVoltageChannels = 0;
//  pCurrentChannels = 0;
//  pSamplerate      = 20000;
//  pMaxCurrent      = 50;
//  pMinCurrent      = 0.1;
//  pChannelMappings = NULL;
//  pActive          = false;
//  pReadPos         = 0;
//  pWritePos        = 0;
//  pUsed            = 0;
//  pBuffLen         = 0;
//  pLastUpdate      = 0;
//}
//void DeviceInterface::AddAnalyzer(AnalyzerInterface *analyzer) {
//  pAnalyzers = (AnalyzerInterface**)realloc(pAnalyzers, sizeof(AnalyzerInterface*) * (pAnalyzersCount) + 1);
//  pAnalyzers[pAnalyzersCount] = analyzer;
//  pAnalyzersCount++;
//  AddAnalyzer(analyzer, 0, 0);
//}
//void DeviceInterface::AddAnalyzer(AnalyzerSoft *analyzer, uint8_t offsetV, uint8_t offsetC) {
//  pAnalyzers = (AnalyzerSoft**)realloc(pAnalyzers, sizeof(AnalyzerSoft*) * (pAnalyzersCount) + 1);
//  pChannelMappings = (ChannelMap_t*)realloc(pChannelMappings, sizeof(ChannelMap_t*) * (pAnalyzersCount) + 1);
//  pAnalyzers[pAnalyzersCount] = analyzer;
//  pChannelMappings[pAnalyzersCount].OffsetV = offsetV;
//  pChannelMappings[pAnalyzersCount].OffsetC = offsetC;
//  pAnalyzersCount++;
//}
//float DeviceInterface::GetSamplerate() {
//  return pSamplerate;
//}
//float DeviceInterface::GetMaxCurrent() {
//  return pMaxCurrent;
//}
//uint32_t DeviceInterface::GetAnalyzers() {
//  return pAnalyzersCount;
//}
//AnalyzerInterface *DeviceInterface::GetAnalyzers(uint32_t idx) {
//  if(idx < pAnalyzersCount) return pAnalyzers[idx];
//  return NULL;
//}
//bool DeviceInterface::BuffersAlloc(uint8_t voltageChannels, uint8_t currentChannels, float samplerate) {
//  pBuffLen = DEVICEINTERFACE_BUFFERSECONDS * samplerate;
//  pVoltageSamples = (float*)malloc(sizeof(float) * pBuffLen * voltageChannels);
//  pCurrentSamples = (float*)malloc(sizeof(float) * pBuffLen * currentChannels);
//  pVoltageChannels = voltageChannels;
//  pCurrentChannels = currentChannels;
//  pSamplerate      = samplerate;
//  return (pVoltageSamples != 0) && (pCurrentSamples != 0);
//}
//void DeviceInterface::BuffersReset() {
//  pUsed     = 0;
//  pWritePos = 0;
//  pReadPos  = 0;
//}
//bool DeviceInterface::BuffersAppend(float *voltageSamples, float *currentSamples) {
//  if(pUsed == pBuffLen) return false;
//  for(int8_t Channel = 0; Channel < pVoltageChannels; Channel++) pVoltageSamples[pWritePos * pVoltageChannels + Channel] = voltageSamples[Channel];
//  for(int8_t Channel = 0; Channel < pCurrentChannels; Channel++) pCurrentSamples[pWritePos * pCurrentChannels + Channel] = currentSamples[Channel];
//  pWritePos = (pWritePos + 1) % pBuffLen;
//  pUsed++;
//  uint64_t Sampletime = Tools::GetTime();
//  if(pLastUpdate < (Sampletime - 500)) {
//    char ThreadName[16];
//    sprintf(ThreadName, "%s_%03d", pDriverName, pUsed * 100 / pBuffLen);
//    pthread_setname_np(pCaptureThread, ThreadName);
//    pLastUpdate = Sampletime;
//  }
//  for(uint8_t Analyzer = 0; Analyzer < pAnalyzersCount; Analyzer++) {
//    if(pAnalyzers[Analyzer]->IsFinished() == false) return true;
//  }
//  bool Started = false;
//  for(uint8_t Sample = 0; (Sample < 5) && (pUsed > 0) && (Started == false); Sample++) {
//    for(uint8_t Analyzer = 0; Analyzer < pAnalyzersCount; Analyzer++) {
//      if(pAnalyzers[Analyzer]->SampleAppend(&pVoltageSamples[pReadPos * pVoltageChannels + pChannelMappings[Analyzer].OffsetV], &pCurrentSamples[pReadPos * pCurrentChannels + pChannelMappings[Analyzer].OffsetC], Sampletime) == true) Started = true;
//    }
//    pReadPos = (pReadPos + 1) % pBuffLen;
//    pUsed--;
//  }
//  return true;
//}


//  free(pChannelMappings);
//  if(pVoltageSamples) free(pVoltageSamples);
//  if(pCurrentSamples) free(pCurrentSamples);