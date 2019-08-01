/* ================================== zMeter ===========================================================================
 * File:     AnalyzerRemote.cpp
 * Author:   Eduardo Viciana (agosto de 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <condition_variable>
#include "Tools.h"
#include "AnalyzerRemote.h"
AnalyzerRemote::AnalyzerRemote(string id) : Analyzer(id) {
  pName = "Never seen";
  pQuerySequence = 0;
}
AnalyzerRemote::~AnalyzerRemote() {
  pQueriesLock.WriteLock();
//  for(list<Query_t*>::iterator it = pQueries.begin(); it != pQueries.end(); it++) pthread_cond_signal(&(*it)->Cond);
  pQueriesLock.WriteUnlock();
}
void AnalyzerRemote::HandleRequest(string url, HTTPRequest &req) {
  Query_t Query;
  Query.Sequence = 0;
  Query.RequestContents = req.GetParams();
  Query.RequestContents.erase("Analyzer");
  pQueriesLock.WriteLock();
  pQueries.push_back(&Query);
  pQueriesLock.WriteUnlock();
  SendQuery();
  if(Query.Locks.ConsumerWait(20000) == false) {
    req.FromString("", MHD_HTTP_GATEWAY_TIMEOUT);
  } else {
    req.FromString(Query.ResponseData, Query.ResponseCode);
    Query.Locks.ConsumerEnds();
  }
  pQueriesLock.WriteLock();
  for(list<Query_t*>::iterator it = pQueries.begin(); it != pQueries.end(); it++) {
    if(*it != &Query) continue;
    pQueries.erase(it);
    break;
  }
  pQueriesLock.WriteUnlock();
}
void AnalyzerRemote::SendQuery() {
  pRequestLocks.WriteLock();
//  if(pRequest.size() == 0) {
//    pRequestLocks.WriteUnlock();
//    return;
//  }
  json Request = json::array();
  pQueriesLock.WriteLock();
  for(list<Query_t*>::iterator it = pQueries.begin(); it != pQueries.end(); it++) {
    if((*it)->Sequence != 0) continue;
    (*it)->Sequence = ++pQuerySequence;
    json Query = json::object();
    Query["SequenceID"] = (*it)->Sequence;
    Query["Contents"] = (*it)->RequestContents;
    Request.push_back(Query);
  }
  LOGFile::Warning("Total: %u, Consulta: %s\n", pQueries.size(), Request.dump().c_str());
  pQueriesLock.WriteUnlock();
  
  
  
  pRequestLocks.WriteUnlock();
}
void AnalyzerRemote::GetRemote(HTTPRequest& req) {
  json Params = req.GetParams();

}
void AnalyzerRemote::SetRemote(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Sequence"].is_number_unsigned()) return req.FromString("'Sequence' param is required", MHD_HTTP_BAD_REQUEST);
  
  
  //Tengo que hacer el pQuery.Locks.ConsumerEnds();
}
void AnalyzerRemote::GetAnalyzer(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;  
  HandleRequest("/GetAnalyzer", req);
}
void AnalyzerRemote::SetAnalyzer(HTTPRequest &req) {
  if(CheckRights(req, CONFIG) == false) return;
  HandleRequest("/SetAnalyzer", req);
}
void AnalyzerRemote::GetSeries(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  HandleRequest("/GetSeries", req);
}
void AnalyzerRemote::GetSeriesNow(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  HandleRequest("/GetSeriesNow", req);
}
void AnalyzerRemote::GetSeriesRange(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  HandleRequest("/GetSeriesRange", req);
}
void AnalyzerRemote::GetEvents(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  HandleRequest("/GetEvents", req);
}
void AnalyzerRemote::GetEvent(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  HandleRequest("/GetEvent", req);
}
void AnalyzerRemote::SetEvent(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  HandleRequest("/SetEvent", req);
}
void AnalyzerRemote::DelEvent(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  HandleRequest("/DelEvent", req);
}
void AnalyzerRemote::GetRates(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  HandleRequest("/GetRates", req);
}
void AnalyzerRemote::GetRate(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  HandleRequest("/GetRate", req);
}
void AnalyzerRemote::SetRate(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  HandleRequest("/SetRate", req);
}
void AnalyzerRemote::DelRate(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  HandleRequest("/DelRate", req);
}
void AnalyzerRemote::AddRate(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  HandleRequest("/AddRate", req);
}
void AnalyzerRemote::GetPrices(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  HandleRequest("/GetPrices", req);
}


//void AnalyzerRemote::GetParams(Params_t& params, ParamsType& copy) {
//}
//void AnalyzerRemote::UpdateConfig(float nominalVoltage, float nominalFreq, char* name, float maxCurrent, float minCurrent, AnalyzerMode mode, float sampleFreq, bool online) {
  //Si ha cambiado el modo tengo que asegurarme que las muestras que hay son coherentes, si no la web fallara!
//}
//void AnalyzerRemote::GetParams(Params_t &params, bool frequency, bool flag, bool rms_v, bool rms_i, bool active_power, bool reactive_power, bool active_energy,
//                                     bool reactive_energy, bool apparent_power, bool power_factor, bool thd_v, bool thd_i, bool phi, bool voltage_harmonics, 
//                                     bool current_harmonics, bool fft_v, bool fft_i, bool voltage, bool current) {
//
//}
//bool AnalyzerRemote::SaveJSON(json_object *json, Database *db) {
//  char *Table = NULL;
//  Tools::JSONString(json, "Table", &Table, false);
//  if(Table == NULL) return false;
//  if((strcmp("aggreg_3s", Table) == 0) || (strcmp("aggreg_3s", Table) == 0) || (strcmp("aggreg_3s", Table) == 0) || (strcmp("aggreg_3s", Table) == 0) || (strcmp("aggreg_3s", Table) == 0)) {
//    Params_t Params;
//    if(strcmp("aggreg_3s", Table) == 0) Params.Type = AGGREG_3S;
//    if(Tools::JSONUInt64(json, "sampletime", &Params.Timestamp) == false) return false;                                                 //sampletime
//    if(Tools::JSONArrayFloat(json, "rms_v", &Params.RMS_V[0], (pMode == PHASE1) ? 1 : 3) == false) return false;                            //rms_v 
//    if(Tools::JSONArrayFloat(json, "rms_i", &Params.RMS_I[0], (pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4) == false) return false;    //rms_i 
//    if(Tools::JSONArrayFloat(json, "active_power", &Params.Active_Power[0], (pMode == PHASE1) ? 1 : 3) == false) return false;              //active_power 
//    if(Tools::JSONArrayFloat(json, "active_energy", &Params.Active_Energy[0], (pMode == PHASE1) ? 1 : 3) == false) return false;            //active_energy
//  BindArrayFloat(params->Reactive_Power, (pMode == PHASE1) ? 1 : 3);                                         //reactive_power
//  BindArrayFloat(params->Reactive_Energy, (pMode == PHASE1) ? 1 : 3);                                        //reactive_energy
//  BindSingleFloat(params->Frequency);                                                                        //frequency
//  BindSingleBool(params->Flag);                                                                              //flaged
//  ExecSentence();
//  if(params->Type == AGGREG_1M) {
//    PrepareSentence("INSERT INTO aggreg_ext_1m(sampletime_ext, power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i) VALUES(?, ?, ?, ?, ?, ?, ?)");
//    BindSingleUInt64(params->Timestamp);                                                                     //sampletime
//    BindArrayFloat(params->Power_Factor, (pMode == PHASE1) ? 1 : 3);                                         //power_factor
//    BindArrayFloat(params->THD_V, (pMode == PHASE1) ? 1 : 3);                                                //thd_v
//    BindArrayFloat(params->THD_I, (pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);                        //thd_i
//    BindArrayFloat(params->Phi, (pMode == PHASE1) ? 1 : 3);                                                  //phi
//    BindArrayArrayFloat((float*)params->VoltageHarmonics, (pMode == PHASE1) ? 1 : 3, 50);                         //harmonics_v
//    BindArrayArrayFloat((float*)params->CurrentHarmonics, (pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, 50); //harmonics_i
//    ExecSentence();
//    PrepareSentence("DELETE FROM aggreg_ext_1m WHERE sampletime_ext < ?");
//    BindSingleUInt64(params->Timestamp - 86400000);                                                          //sampletime
//    ExecSentence();
//  }
//  if(params->Type == AGGREG_1H) {
//    PrepareSentence("INSERT INTO aggreg_stat_1h(sampletime_ext, stat_voltage, stat_frequency, stat_thd, stat_harmonics) VALUES(?, ?, ?, ?, ?)");
//    BindSingleUInt64(params->Timestamp);
//    BindArrayUInt16(params->StatsVoltage, 26);     //estadisticas de tension
//    BindArrayUInt16(params->StatsFrequency, 40); //estadisticas de frequencia
//    BindArrayUInt16(params->StatsTHD, 26);             //estadisticas de distorsion harmonica
//    BindArrayUInt16(params->StatsHarmonics, 2);                                           //estadisticas de harmonicos en rango
//    ExecSentence();
//  }

    
    
//    
//    db->PrepareSentence("INSERT INTO aggreg_3s SELECT * from json_populate_record(null::\"aggreg_3s\", ?)");
//    db->BindSingleString(json_object_to_json_string(reader));
//    if(db->ExecSentence() == false) {
//      *postdata  = (char*)"Can save data";
//      *resultLen = strlen(*postdata);
//      return false;
//    } else {
//      *writer = json_object_new_object();
//      json_object_object_add(*writer, "RequiereLive", json_object_new_boolean(false));
//      *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
//      return true;
//    }
//    SaveAggreg(Params);
//    LOGFile::Debug("Storing 3S aggregation\n");
//  } 
  
  
  

//  char *Table = NULL;
//  if((json_object_object_get_ex(json, "Table", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Table = (char*)json_object_get_string(Find);
//  db->PrepareSentence("SET search_path TO ?");
//  db->BindSingleString(pID);
//  db->ExecSentence();
//  if(strcmp("aggreg_3s", Table) == 0) {
//    db->PrepareSentence("INSERT INTO SELECT * from json_populate_record(null::\"aggreg_3s\", ?)");
//    db->BindSingleString(json_object_to_json_string(json));
//    return db->ExecSentence();
//  }  
  
//  
//  if((json_object_object_get_ex(config, "aggreg", &Find) == true) && (json_object_get_type(Find) == json_type_string)) {
//    if(ReadJSONFloatArray(Find, pVoltageGain, 3) == false) {
//      LOGFile::Error("CaptureV2: Voltage gain must have 3 double values\n");
//      return;
//    }
//  }
//  
//    if(json_object_array_length(object) != len) return false;
//  for(int i = 0; i < len; i++) {
//    if(json_object_get_type(json_object_array_get_idx(object, i)) != json_type_double) return false;
//    dst[i] = json_object_get_double(json_object_array_get_idx(object, i));
//  }
//  return true;
//  
//  
//  
//  
//  
//  
//  
//        AggregType Type;                          //Tipo de agregacion
//      uint64_t   Timestamp;                     //Timestamp de la muestra inicial
//      uint32_t   Samples;                       //Muestras que han formado esta agregacion
//      float      RMS_V[3];                      //Valor RMS de la tension durante el bloque
//      float      RMS_I[4];                      //Valor RMS de la intensidad durante el bloque
//      float      Active_Power[3];               //Potencia activa (P)
//      float      Apparent_Power[3];             //Potencia aparente (S)
//      float      Power_Factor[3];               //Factor de potencia (cos(PHI))
//      float      Active_Energy[3];              //Energia consumida
//      float      Reactive_Power[3];             //Reactive power
//      float      Reactive_Energy[3];            //Reactive energy
//      float      Frequency;                     //Frecuencia
//      bool       Flag;                          //Periodo marcado
//      float      THD_V[3];                      //Distorsion harmonica de la tension
//      float      THD_I[4];                      //Distorsion harmonica de la corriente
//      float      VoltageHarmonics[3][50];       //Armonicos del canal de tension
//      float      CurrentHarmonics[4][50];       //Armonicos del canal de corriente
//      float      Phi[3];                        //Angulo de Phi
//    } Params_t;
  
//  return true;
//
//}