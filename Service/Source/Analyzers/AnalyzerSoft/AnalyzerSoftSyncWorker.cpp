// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#include "AnalyzerSoftSyncWorker.h"
#include "Tools.h"
#include "LogFile.h"
#include "AnalyzerSoft.h"
#include "AnalyzerSoftSync.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftSyncWorker::AnalyzerSoftSyncWorker(AnalyzerSoftSync *parent, AnalyzerSoft *analyzer) {
  pTerminate = false;
  pCond      = PTHREAD_COND_INITIALIZER;
  pMutex     = PTHREAD_MUTEX_INITIALIZER;
  pParent    = parent;
  pAnalyzer  = analyzer;
  pParams    = nullptr;
  pMethod    = "";
  pClient    = NULL;
  if(pthread_create(&pThread, NULL, [](void *params) { return ((AnalyzerSoftSyncWorker*)params)->Thread(); }, (void*)this) != 0) {
    pThread = 0;
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return;
  }
  pthread_setname_np(pThread, "SyncWorker");
}
AnalyzerSoftSyncWorker::~AnalyzerSoftSyncWorker() {
  pthread_mutex_lock(&pMutex);
  pTerminate = true;
  pthread_cond_signal(&pCond);
  pthread_mutex_unlock(&pMutex);
  pthread_join(pThread, NULL);
}
uint64_t AnalyzerSoftSyncWorker::LastUse() const {
  return pLastTime;
}
void AnalyzerSoftSyncWorker::Run(const string &cookie, const URL &url, const string &sequence, const string &method, const json &params) {
  pthread_mutex_lock(&pMutex);
  pParams   = params;
  pSequence = sequence;
  pMethod   = method;
  pCookie   = cookie;
  pPath     = url.Path();
  if((pPort != url.Port()) || (pHost != url.Host()) || (pClient == NULL)) {
    pPort = url.Port();
    pHost = url.Host();
    ResetClient();
  }
  pthread_cond_signal(&pCond);
  pthread_mutex_unlock(&pMutex);
}
void  AnalyzerSoftSyncWorker::ResetClient() {
  if(pClient != NULL) {
    delete pClient;
    pClient = NULL;
  }
  pClient = new Client(pHost, pPort);
  pClient->set_keep_alive(false);
  pClient->set_compress(false);
  pClient->set_read_timeout(30);
  pClient->set_write_timeout(30);
  pClient->set_connection_timeout(30);
}
void *AnalyzerSoftSyncWorker::Thread() {
  nice(15);
  while(true) {
    pthread_mutex_lock(&pMutex);
    while((pTerminate == false) && (pMethod.empty() == true)) pthread_cond_wait(&pCond, &pMutex);
    if(pTerminate == true) return NULL;
    pthread_mutex_unlock(&pMutex);
    pParams["UserName"] = "admin";
    Response Resp;
    Resp.status = HTTP_OK;
    if(pMethod == "getAnalyzer")         pAnalyzer->GetAnalyzer(pParams, Resp);
    else if(pMethod == "setAnalyzer")    pAnalyzer->SetAnalyzer(pParams, Resp);
    else if(pMethod == "getStatus")      pAnalyzer->GetStatus(pParams, Resp);
    else if(pMethod == "getSeries")      pAnalyzer->GetSeries(pParams, Resp);
    else if(pMethod == "getSeriesNow")   pAnalyzer->GetSeriesNow(pParams, Resp);
    else if(pMethod == "getCosts")       pAnalyzer->GetCosts(pParams, Resp);
    else if(pMethod == "getBills")       pAnalyzer->GetBills(pParams, Resp);
    else if(pMethod == "getWaveStream")  pAnalyzer->GetWaveStream(pParams, Resp);
    else if(pMethod == "getSeriesRange") pAnalyzer->GetSeriesRange(pParams, Resp);
    else if(pMethod == "delSeriesRange") pAnalyzer->DelSeriesRange(pParams, Resp);
    else if(pMethod == "getEvents")      pAnalyzer->GetEvents(pParams, Resp);
    else if(pMethod == "getEventsRange") pAnalyzer->GetEventsRange(pParams, Resp);
    else if(pMethod == "delEventsRange") pAnalyzer->DelEventsRange(pParams, Resp);
    else if(pMethod == "getEvent")       pAnalyzer->GetEvent(pParams, Resp);
    else if(pMethod == "setEvent")       pAnalyzer->SetEvent(pParams, Resp);
    else if(pMethod == "delEvent")       pAnalyzer->DelEvent(pParams, Resp);
    else if(pMethod == "getAlarmsDef")   pAnalyzer->GetAlarmsDef(pParams, Resp);
    else if(pMethod == "testAlarm")      pAnalyzer->TestAlarm(pParams, Resp);
    else if(pMethod == "addAlarmDef")    pAnalyzer->AddAlarmDef(pParams, Resp);
    else if(pMethod == "setAlarmDef")    pAnalyzer->SetAlarmDef(pParams, Resp);
    else if(pMethod == "delAlarmDef")    pAnalyzer->DelAlarmDef(pParams, Resp);
    else if(pMethod == "getAlarms")      pAnalyzer->GetAlarms(pParams, Resp);
    else if(pMethod == "getAlarmsRange") pAnalyzer->GetAlarmsRange(pParams, Resp);
    else if(pMethod == "delAlarm")       pAnalyzer->DelAlarm(pParams, Resp);
    else if(pMethod == "delAlarmsRange") pAnalyzer->DelAlarmsRange(pParams, Resp);
    else if(pMethod == "getRates")       pAnalyzer->GetRates(pParams, Resp);
    else if(pMethod == "getRate")        pAnalyzer->GetRate(pParams, Resp);
    else if(pMethod == "addRate")        pAnalyzer->AddRate(pParams, Resp);
    else if(pMethod == "setRate")        pAnalyzer->SetRate(pParams, Resp);
    else if(pMethod == "delRate")        pAnalyzer->DelRate(pParams, Resp);
    else if(pMethod == "getPermissions") pAnalyzer->GetPermissions(pParams, Resp);
    else if(pMethod == "addPermission")  pAnalyzer->AddPermission(pParams, Resp);
    else if(pMethod == "setPermission")  pAnalyzer->SetPermission(pParams, Resp);
    else if(pMethod == "delPermission")  pAnalyzer->DelPermission(pParams, Resp);
    Headers Header  = {{"Accept", "application/json"}, {"Cookie", pCookie}, {"X-Analyzer", pAnalyzer->Serial().c_str()}, {"X-Sequence", pSequence.c_str()}, {"X-Status", to_string(Resp.status).c_str()}};
    for(uint8_t Retry = 10; Retry > 0; Retry--) {
      auto Tmp = (Resp.is_chunked_content_provider_ == false) ?
        pClient->Post((pPath + "/setRemote").c_str(), Header, Resp.body, Resp.has_header("Content-Type") ? Resp.get_header_value("Content-Type").c_str() : "text/plain") :
        pClient->Post((pPath + "/setRemote").c_str(), Header, [&](size_t, DataSink &s) { return Resp.content_provider_(0, 0, s); }, Resp.has_header("Content-Type") ? Resp.get_header_value("Content-Type").c_str() : "text/plain");
      if((Tmp != nullptr) && (Tmp->status == HTTP_OK)) {
        LogFile::Debug("Remote request '%s' for analyzer '%s' completed", pMethod.c_str(), pAnalyzer->Serial().c_str());
        break;
      } else if(Tmp != nullptr) {
        if(Retry == 1) LogFile::Warning("Remote request '%s' for analyzer '%s' failed (ERROR: %d)", pMethod.c_str(), pAnalyzer->Serial().c_str(), Tmp->status);
        ResetClient();
      } else {
        if(Retry == 1) LogFile::Warning("Remote request '%s' for analyzer '%s' failed (can not connect)", pMethod.c_str(), pAnalyzer->Serial().c_str());
        ResetClient();
      }
    }
    pthread_mutex_lock(&pMutex);
    pMethod = "";
    pLastTime = Tools::GetTime64();
    pthread_mutex_unlock(&pMutex);
    pParent->PushWorker(this, false);
  }
}
// ---------------------------------------------------------------------------------------------------------------------