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
AnalyzerRemote::AnalyzerRemote(string id) : Analyzer(id) {
  SetName("Never seen");
  pPendingRequestLocks = PTHREAD_MUTEX_INITIALIZER;
}
AnalyzerRemote::~AnalyzerRemote() {
//  pMembersLocks.WriteLock();
//  if(pRequestLocks != NULL) pRequestLocks->Abort();
//  for(list<Query_t*>::iterator it = pQueries.begin(); it != pQueries.end(); it++) (*it)->Locks.Abort();
//  pMembersLocks.WriteUnlock();
}
void AnalyzerRemote::GetRemote(const json &params, Response &response) {
//  json Params = req.GetParams();
//  json Response = json::array();
//  pMembersLocks.WriteLock();
//  pLastSave = Tools::GetTime();
//  pName     = Params["Name"];
//  if(pQueries.size() > 0) {
//    for(list<Query_t*>::iterator it = pQueries.begin(); it != pQueries.end(); it++) {
//      if((*it)->Sequence != 0) continue;
//      (*it)->Sequence = ++pQuerySequence;
//      json Query = json::object();
//      Query["Method"] = (*it)->Method;
//      Query["Params"] = (*it)->Request->GetParams();
//      Query["Params"].erase("Analyzer");
//      Query["Sequence"] = (*it)->Sequence;
//      Response.push_back(Query);
//    }
//    pMembersLocks.WriteUnlock();
//    return req.FromJSON(Response);
//  } else {
//    if(pRequestLocks != NULL) {
//      pMembersLocks.WriteUnlock();
//      return req.FromString("", MHD_HTTP_SERVICE_UNAVAILABLE);
//    } else {
//      pRequestLocks = new ProviderLocks();
//      pMembersLocks.WriteUnlock();
//      if(pRequestLocks->ConsumerWait(15000) == false) {
//        pMembersLocks.WriteLock();
//        req.FromString("", MHD_HTTP_NO_CONTENT);
//      } else {
//        pMembersLocks.WriteLock();
//        for(list<Query_t*>::iterator it = pQueries.begin(); it != pQueries.end(); it++) {
//          if((*it)->Sequence != 0) continue;
//          (*it)->Sequence = ++pQuerySequence;
//          json Query = json::object();
//          Query["Method"] = (*it)->Method;
//          Query["Params"] = (*it)->Request->GetParams();
//          Query["Params"].erase("Analyzer");
//          Query["Sequence"] = (*it)->Sequence;
//          Response.push_back(Query);
//        }
//        req.FromJSON(Response);
//      }
//      delete pRequestLocks;
//      pRequestLocks = NULL;
//      pMembersLocks.WriteUnlock();
//    }
//  }
}
void AnalyzerRemote::SetRemote(const json &params, Response &response) {
//  json Params = req.GetParams();
//  if(!Params["Code"].is_number() || !Params["Sequence"].is_number_unsigned()) return req.FromString("'Sequence' and 'Code' param are required", MHD_HTTP_BAD_REQUEST);
//  uint64_t Sequence = Params["Sequence"];
//  uint16_t Code = Params["Code"];
//  pMembersLocks.WriteLock();
//  for(list<Query_t*>::iterator it = pQueries.begin(); it != pQueries.end(); it++) {
//    if((*it)->Sequence != Sequence) continue;
//    Query_t *Query = (*it);
//    pQueries.erase(it);
//    pMembersLocks.WriteUnlock();
//    Query->Request->FromString(Params["Response"], Code);
//    Query->Locks.ProviderWrite();
//    return;
//  }
//  pMembersLocks.WriteUnlock();
}
void AnalyzerRemote::HandleRequest(const string &url, const json &params, Response &response) {
  Request_t Request;
  Request.RequestMethod = &url;
  Request.RequestParams = &params;
  Request.Response      = &response;
  Request.Sended        = false;
  pthread_mutex_lock(&pLocks);
  uint64_t Sequence = pSequence++;

  pPendingRequest.insert({ Sequence, &Request });
//  if(pRequestLocks != NULL) pRequestLocks->ProviderWrite();
//  pMembersLocks.WriteUnlock();
//  if(Query.Locks.ConsumerWait(15000) == false) req.FromString("", MHD_HTTP_GATEWAY_TIMEOUT);
//  pMembersLocks.WriteLock();
  if(Request.Sended == true) {
    pSendedRequest.erase(Sequence);
  } else {
    pPendingRequest.erase(Sequence);
  }
//  for(list<Query_t*>::iterator it = pQueries.begin(); it != pQueries.end(); it++) {
//    if(*it != &Query) continue;
//    pQueries.erase(it);
//    break;
//  }
//  pMembersLocks.WriteUnlock();


  pthread_mutex_unlock(&pLocks);

}
void AnalyzerRemote::GetAnalyzer(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getAnalyzer", params, response);
}
void AnalyzerRemote::SetAnalyzer(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("setAnalyzer", params, response);
}
void AnalyzerRemote::GetSeries(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getSeries", params, response);
}
void AnalyzerRemote::GetSeriesNow(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getSeriesNow", params, response);
}
void AnalyzerRemote::GetSeriesRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getSeriesRange", params, response);
}
void AnalyzerRemote::GetEvents(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getEvents", params, response);
}
void AnalyzerRemote::GetEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getEvent", params, response);
}
void AnalyzerRemote::SetEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("setEvent", params, response);
}
void AnalyzerRemote::DelEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("delEvent", params, response);
}
void AnalyzerRemote::GetRates(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getRates", params, response);
}
void AnalyzerRemote::GetRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getRate", params, response);
}
void AnalyzerRemote::SetRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("setRate", params, response);
}
void AnalyzerRemote::DelRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("delRate", params, response);
}
void AnalyzerRemote::AddRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("addRate", params, response);
}
void AnalyzerRemote::GetPrices(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getPrices", params, response);
}
void AnalyzerRemote::TestAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("testAlarm", params, response);
}
void AnalyzerRemote::GetAlarms(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getAlarms", params, response);
}
void AnalyzerRemote::GetAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("getAlarm", params, response);
}
void AnalyzerRemote::SetAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("setAlarm", params, response);
}
void AnalyzerRemote::AddAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("addAlarm", params, response);
}
void AnalyzerRemote::DelAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  HandleRequest("delAlarm", params, response);
}