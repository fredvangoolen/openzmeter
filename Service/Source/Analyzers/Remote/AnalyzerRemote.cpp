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
#include <string.h>
#include <condition_variable>
#include "Tools.h"
#include "AnalyzerRemote.h"
AnalyzerRemote::AnalyzerRemote(const string &id) : Analyzer(id) {
  SetName("Never seen");
  pMutex           = PTHREAD_MUTEX_INITIALIZER;
  pProducerCond    = PTHREAD_COND_INITIALIZER;
  pConsumerCond    = PTHREAD_COND_INITIALIZER;
  pTerminate       = false;
  pConsumerReady   = false;
  pProducerRequest = NULL;
  pSequence        = 0;
}
AnalyzerRemote::~AnalyzerRemote() {
  pTerminate = true;
  //Espero.. no se como ni a que!

//  pMembersLocks.WriteLock();
//  if(pRequestLocks != NULL) pRequestLocks->Abort();
//  for(list<Query_t*>::iterator it = pQueries.begin(); it != pQueries.end(); it++) (*it)->Locks.Abort();
//  pMembersLocks.WriteUnlock();
}
void AnalyzerRemote::GetRemote(const json &params, Response &response) {
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  SetName(params["Name"]);
  response.set_header("Content-Type", "text/event-stream");
  response.set_chunked_content_provider(
    [&](uint64_t offset, DataSink &sink) {
      if((pTerminate == true) || (sink.is_writable() == false)) return sink.done();
      SetOnline();
      pthread_mutex_lock(&pMutex);
      pConsumerReady = true;
      int Result     = 0;
      if(pProducerRequest == NULL) {
        timespec Time;
        clock_gettime(CLOCK_REALTIME, &Time);
        Time.tv_sec = Time.tv_sec + 1;
        Result = pthread_cond_timedwait(&pConsumerCond, &pMutex, &Time);
      }
      if(Result == ETIMEDOUT) {
        string Data = "data: null\n\n";
        sink.write(Data.c_str(), Data.size());
      } else if(Result == 0) {
        pRequestList.insert(pProducerRequest);
        string Data = "data: " + pProducerRequest->Request.dump() + "\n\n";
        sink.write(Data.c_str(), Data.size());
        pProducerRequest = NULL;
      } else {
        LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      }
      pthread_cond_signal(&pProducerCond);
      pthread_mutex_unlock(&pMutex);
    },
    [&]() {
      pthread_mutex_lock(&pMutex);
      pConsumerReady = false;
      pthread_cond_broadcast(&pProducerCond);
      pthread_mutex_unlock(&pMutex);
    }
  );
}
void AnalyzerRemote::SetRemote(const json &params, Response &response) {
  if((params.contains("Code") == false) || (params["Code"].is_number() == false)) return HTTPServer::FillResponse(response, "'Code' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Sequence") == false) || (params["Sequence"].is_number() == false)) return HTTPServer::FillResponse(response, "'Sequence' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Response") == false) || (params["Response"].is_string() == false)) return HTTPServer::FillResponse(response, "'Response' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Mime") == false) || (params["Mime"].is_string() == false)) return HTTPServer::FillResponse(response, "'Mime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  pthread_mutex_lock(&pMutex);
  uint64_t Sequence = params["Sequence"];
  auto     Request  = pRequestList.find(Sequence);
  if(Request == pRequestList.end()) {
    pthread_mutex_unlock(&pMutex);
    return HTTPServer::FillResponse(response, "Sequence not found", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  (*Request)->Response = json::object();
  (*Request)->Response["Code"] = params["Code"];
  (*Request)->Response["Mime"] = params["Mime"];
  (*Request)->Response["Response"] = params["Response"];
  pthread_cond_signal(&(*Request)->Cond);
  pthread_mutex_unlock(&pMutex);
  return HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void AnalyzerRemote::PeriodicTasks() {
}
void AnalyzerRemote::HandleRequest(const string &url, const json &params, Response &response) {
  pthread_mutex_lock(&pMutex);
  if(pConsumerReady == false) {
    pthread_mutex_unlock(&pMutex);
    return HTTPServer::FillResponse(response, "", "text/plain", HTTP_SERVICE_UNAVAILABLE);
  }
  while(pProducerRequest != NULL) {
    pthread_cond_wait(&pProducerCond, &pMutex);
    if(pConsumerReady == false) {
      pthread_mutex_unlock(&pMutex);
      return HTTPServer::FillResponse(response, "", "text/plain", HTTP_SERVICE_UNAVAILABLE);
    }
  }
  Request_t Request;
  timespec  Time;
  Request.Cond     = PTHREAD_COND_INITIALIZER;
  Request.Sequence = pSequence++;
  Request.Request  = json::object();
  Request.Request["Method"]   = url;
  Request.Request["Params"]   = params;
  Request.Request["Sequence"] = Request.Sequence;
  Request.Request["Params"].erase("Analyzer");
  Request.Response = nullptr;
  pProducerRequest = &Request;
  pthread_cond_signal(&pConsumerCond);
  clock_gettime(CLOCK_REALTIME, &Time);
  Time.tv_sec = Time.tv_sec + 30;
  int Result  = pthread_cond_timedwait(&Request.Cond, &pMutex, &Time);
  auto Tmp    = pRequestList.find(Request.Sequence);
  if(Tmp != pRequestList.end()) pRequestList.erase(Tmp);
  if(Result == ETIMEDOUT) {
    pthread_mutex_unlock(&pMutex);
    return HTTPServer::FillResponse(response, "", "text/plain", HTTP_GATEWAY_TIMEOUT);
  } else if(Result == 0) {
    pthread_mutex_unlock(&pMutex);
    string Mime = Request.Response["Mime"];
    int    Code = Request.Response["Code"];
    return HTTPServer::FillResponse(response, Request.Response["Response"], Mime.c_str(), Code);
  } else {
    pthread_mutex_unlock(&pMutex);
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
}
void AnalyzerRemote::GetAnalyzer(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getAnalyzer", params, response);
}
void AnalyzerRemote::SetAnalyzer(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("setAnalyzer", params, response);
}
void AnalyzerRemote::GetSeries(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getSeries", params, response);
}
void AnalyzerRemote::GetSeriesNow(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getSeriesNow", params, response);
}
void AnalyzerRemote::GetWaveStream(const json &params, Response &response) {

}
void AnalyzerRemote::GetSeriesRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getSeriesRange", params, response);
}
void AnalyzerRemote::GetEvents(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getEvents", params, response);
}
void AnalyzerRemote::GetEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getEvent", params, response);
}
void AnalyzerRemote::SetEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("setEvent", params, response);
}
void AnalyzerRemote::DelEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("delEvent", params, response);
}
void AnalyzerRemote::GetPrices(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getPrices", params, response);
}
void AnalyzerRemote::TestAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("testAlarm", params, response);
}
void AnalyzerRemote::GetAlarms(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getAlarms", params, response);
}