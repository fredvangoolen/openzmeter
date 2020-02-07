#include "Sync.h"
#include <iostream>
//------------------------------------------------------------------------------
AnalyzerSoft::Sync::Sync(AnalyzerSoft *parent) {
  pCookie       = "";
  pThreadsCount = 0;
}
AnalyzerSoft::Sync::~Sync() {
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  pLocks.ReadLock();
  while(pThreadsCount > 0) {
    pLocks.ReadUnlock();
    usleep(100000);
    pLocks.ReadLock();
  }
  pLocks.ReadUnlock();
}
bool AnalyzerSoft::Sync::Logged() {
  pLocks.ReadLock();
  bool Result = !pCookie.empty();
  pLocks.ReadUnlock();
  return Result;
}
void *AnalyzerSoft::Sync::Sync_Thread(void *args) {
}
void *AnalyzerSoft::Sync::Request_Thread(void *args) {
  Request_t *This = (Request_t*)args;
  This->Params["UserName"] = "admin";
//  string ParamsText = This->Params.dump();
//  size_t ParamsSize = ParamsText.size();
  LOGFile::Debug("Remote server request query '%s' for analyzar '%s'\n", This->Method.c_str(), This->Analyzer->Serial.c_str());
//  HTTPRequest Request(ParamsText.c_str());
//  Request.AppendParams(ParamsText.c_str(), &ParamsSize);
//  Request.ProcessParams(true);
  Response Resp;
  if(This->Method == "getAnalyzer") {
    This->Analyzer->GetAnalyzer(This->Params, Resp);
  } else if(This->Method == "setAnalyzer") {
    This->Analyzer->SetAnalyzer(This->Params, Resp);
  } else if(This->Method == "getSeries") {
    This->Analyzer->GetSeries(This->Params, Resp);
  } else if(This->Method == "getSeriesNow") {
    This->Analyzer->GetSeriesNow(This->Params, Resp);
  } else if(This->Method == "getSeriesRange") {
    This->Analyzer->GetSeriesRange(This->Params, Resp);
  } else if(This->Method == "getEvents") {
    This->Analyzer->GetEvents(This->Params, Resp);
  } else if(This->Method == "getEvent") {
    This->Analyzer->GetEvent(This->Params, Resp);
  } else if(This->Method == "setEvent") {
    This->Analyzer->SetEvent(This->Params, Resp);
  } else if(This->Method == "delEvent") {
    This->Analyzer->DelEvent(This->Params, Resp);
  } else if(This->Method == "getRates") {
    This->Analyzer->GetRates(This->Params, Resp);
  } else if(This->Method == "getRate") {
    This->Analyzer->GetRate(This->Params, Resp);
  } else if(This->Method == "setRate") {
    This->Analyzer->SetRate(This->Params, Resp);
  } else if(This->Method == "delRate") {
    This->Analyzer->DelRate(This->Params, Resp);
  } else if(This->Method == "addRate") {
    This->Analyzer->AddRate(This->Params, Resp);
  } else if(This->Method == "getPrices") {
    This->Analyzer->GetPrices(This->Params, Resp);
  } else if(This->Method == "testAlarm") {
    This->Analyzer->TestAlarm(This->Params, Resp);
  } else if(This->Method == "getAlarms") {
    This->Analyzer->GetAlarms(This->Params, Resp);
  } else if(This->Method == "getAlarm") {
    This->Analyzer->GetAlarm(This->Params, Resp);
  } else if(This->Method == "setAlarm") {
    This->Analyzer->SetAlarm(This->Params, Resp);
  } else if(This->Method == "addAlarm") {
    This->Analyzer->AddAlarm(This->Params, Resp);
  } else if(This->Method == "delAlarm") {
    This->Analyzer->DelAlarm(This->Params, Resp);
  }
  json Content = json::object();
  Content["Code"] = Resp.status;
  Content["Sequence"] = This->Sequence;
  Content["Analyzer"] = This->Analyzer->Serial;
  Content["Response"] = Resp.body;
  This->Sync->pLocks.ReadLock();
  Headers Header = {{"Accept", "application/json"}, {"Connection", "keep-alive"}, {"Cookie", This->Sync->pCookie}};
  string  Path   = This->Sync->pPath + "/setRemote";
  Client HTTPClient(This->Sync->pServer.c_str(), This->Sync->pPort);
  This->Sync->pLocks.ReadUnlock();
  auto Response = HTTPClient.Post(Path.c_str(), Header, Content.dump(), "application/json", true);
  if((Response != nullptr) && (Response->status == HTTP_OK)) {
    LOGFile::Debug("Remote request '%s' for analyzer '%s' completed\n", This->Method.c_str(), This->Analyzer->Serial.c_str());
  } else {
    LOGFile::Error("Remote request '%s' for analyzer '%s' failed\n", This->Method.c_str(), This->Analyzer->Serial.c_str());
  }
  This->Sync->pLocks.WriteLock();
  This->Sync->pThreadsCount--;
  This->Sync->pLocks.WriteUnlock();
  delete This;
  return NULL;
}
