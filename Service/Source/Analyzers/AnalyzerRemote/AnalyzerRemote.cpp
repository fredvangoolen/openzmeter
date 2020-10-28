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
#include "Tools.h"
#include "AnalyzerRemote.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerRemote::AnalyzerRemote(const string &id) : Analyzer(id) {
  Name("Never seen");
  pMutex     = PTHREAD_MUTEX_INITIALIZER;
  pCond      = PTHREAD_COND_INITIALIZER;
  pTerminate = false;
}
AnalyzerRemote::~AnalyzerRemote() {
  pTerminate = true;
  pthread_cond_broadcast(&pCond);
}
void AnalyzerRemote::GetRemote(const json &params, Response &response) {
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Name(params["Name"]);
  auto Func = [&](uint64_t offset, DataSink &sink) {
    while((pTerminate == false) && (sink.is_writable() == true)) {
      Online(true);
      pthread_mutex_lock(&pMutex);
      int Result = 0;
      if(pSendPending.size() == 0) {
        timespec Time;
        clock_gettime(CLOCK_REALTIME, &Time);
        Time.tv_sec = Time.tv_sec + 10;
        Result = pthread_cond_timedwait(&pCond, &pMutex, &Time);
      }
      if((Result == 0) && (pSendPending.size() > 0)) {
        auto Request = pSendPending.begin();
        sink.os << "data: " + Request->second->ReqParams.dump() + "\n\n";
        Request->second->Map = &pResponsePending;
        pResponsePending.insert( { Request->first, Request->second } );
        pSendPending.erase(Request);
      } else if((Result != 0) && (Result != ETIMEDOUT)) {
        LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      } else {
        sink.os << "data: null\n\n";
      }
      pthread_mutex_unlock(&pMutex);
    }
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("text/event-stream", Func);
}
void AnalyzerRemote::SetRemote(const map<string, string> &params, Response &response, const ContentReader &content_reader) {
  pthread_mutex_lock(&pMutex);
  auto Tmp = pResponsePending.find(params.find("X-Sequence")->second);
  if(Tmp == pResponsePending.end()) {
    pthread_mutex_unlock(&pMutex);
    return HTTPServer::FillResponse(response, "Sequence not found", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  Request_t *Request = Tmp->second;
  int        WriteFD = Request->Pipe[1];
  Request->Map       = NULL;
  Request->ResMime   = params.find("Content-Type")->second;
  Request->ResCode   = atoi(params.find("X-Status")->second.c_str());
  pResponsePending.erase(Tmp);
  pthread_cond_signal(&Request->Cond);
  pthread_mutex_unlock(&pMutex);

  content_reader([&, Request](const char *data, size_t length) -> bool {
    ssize_t Write = write(WriteFD, data, length);
    if(Write > 0) return true;
    if(Write == -1) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return false;
  });
  close(WriteFD);
  return HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void AnalyzerRemote::HandleRequest(const Analyzer::AnalyzerRights rights, const string &url, const json &params, Response &response) {
  if(CheckRights(params["UserName"], rights) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  timespec   Time;
  string     Sequence = Tools::GetUUID();
  Request_t *Request  = new Request_t;
  Request->Cond             = PTHREAD_COND_INITIALIZER;
  Request->Map              = &pSendPending;
  Request->ReqParams        = params;
  Request->ReqParams["Method"]   = url;
  Request->ReqParams["Sequence"] = Sequence;
  Request->ReqParams.erase("Analyzer");
  Request->ReqParams.erase("UserName");
  if(pipe(Request->Pipe) != 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    delete Request;
    return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  pthread_mutex_lock(&pMutex);
  pSendPending.emplace(Sequence, Request);
  pthread_cond_signal(&pCond);
  clock_gettime(CLOCK_REALTIME, &Time);
  Time.tv_sec = Time.tv_sec + 10;
  pthread_cond_timedwait(&Request->Cond, &pMutex, &Time);
  if(Request->Map != NULL) {
    Request->Map->erase(Sequence);
    pthread_mutex_unlock(&pMutex);
    close(Request->Pipe[0]);
    close(Request->Pipe[1]);
    delete Request;
    return HTTPServer::FillResponse(response, "", "text/plain", HTTP_GATEWAY_TIMEOUT);
  }
  pthread_mutex_unlock(&pMutex);

  auto Func = [&, Request](uint64_t offset, DataSink &sink) -> bool {
    char Buff[128];
    ssize_t Read = read(Request->Pipe[0], Buff, sizeof(Buff));
    if(Read > 0) {
      sink.write(Buff, Read);
      return true;
    }
    if(Read == -1) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    sink.done();
    return true;
  };
  auto Free = [Request]() {
    close(Request->Pipe[0]);
    delete Request;
  };
  response.status = Request->ResCode;
  response.set_chunked_content_provider(Request->ResMime.c_str(), Func, Free);
}
void AnalyzerRemote::GetAnalyzer(const json &params, Response &response) {
  HandleRequest(READ, "getAnalyzer", params, response);
}
void AnalyzerRemote::SetAnalyzer(const json &params, Response &response) {
  HandleRequest(CONFIG, "setAnalyzer", params, response);
}
void AnalyzerRemote::GetSeries(const json &params, Response &response) {
  HandleRequest(READ, "getSeries", params, response);
}
void AnalyzerRemote::GetSeriesNow(const json &params, Response &response) {
  HandleRequest(READ, "getSeriesNow", params, response);
}
void AnalyzerRemote::GetStatus(const json &params, Response &response) {
  HandleRequest(READ, "getStatus", params, response);
}
void AnalyzerRemote::GetWaveStream(const json &params, Response &response) {
  HandleRequest(READ, "getWaveStream", params, response);
}
void AnalyzerRemote::GetSeriesRange(const json &params, Response &response) {
  HandleRequest(READ, "getSeriesRange", params, response);
}
void AnalyzerRemote::DelSeriesRange(const json &params, Response &response) {
  HandleRequest(WRITE, "delSeriesRange", params, response);
}
void AnalyzerRemote::GetEvents(const json &params, Response &response) {
  HandleRequest(READ, "getEvents", params, response);
}
void AnalyzerRemote::GetEventsRange(const json &params, Response &response) {
  HandleRequest(READ, "getEventsRange", params, response);
}
void AnalyzerRemote::DelEventsRange(const json &params, Response &response) {
  HandleRequest(WRITE, "delEventsRange", params, response);
}
void AnalyzerRemote::GetEvent(const json &params, Response &response) {
  HandleRequest(READ, "getEvent", params, response);
}
void AnalyzerRemote::SetEvent(const json &params, Response &response) {
  HandleRequest(WRITE, "setEvent", params, response);
}
void AnalyzerRemote::DelEvent(const json &params, Response &response) {
  HandleRequest(WRITE, "delEvent", params, response);
}
void AnalyzerRemote::GetCosts(const json &params, Response &response) {
  HandleRequest(READ, "getCosts", params, response);
}
void AnalyzerRemote::GetBills(const json &params, Response &response) {
  HandleRequest(READ, "getBills", params, response);
}
void AnalyzerRemote::GetAlarmsDef(const json &params, Response &response) {
  HandleRequest(READ, "getAlarmsDef", params, response);
}
void AnalyzerRemote::TestAlarm(const json &params, Response &response) {
  HandleRequest(READ, "testAlarm", params, response);
}
void AnalyzerRemote::AddAlarmDef(const json &params, Response &response) {
  HandleRequest(CONFIG, "addAlarmDef", params, response);
}
void AnalyzerRemote::SetAlarmDef(const json &params, Response &response) {
  HandleRequest(CONFIG, "setAlarmDef", params, response);
}
void AnalyzerRemote::DelAlarmDef(const json &params, Response &response) {
  HandleRequest(CONFIG, "delAlarmDef", params, response);
}
void AnalyzerRemote::GetAlarms(const json &params, Response &response) {
  HandleRequest(READ, "getAlarms", params, response);
}
void AnalyzerRemote::GetAlarmsRange(const json &params, Response &response) {
  HandleRequest(READ, "getAlarmsRange", params, response);
}
void AnalyzerRemote::DelAlarmsRange(const json &params, Response &response) {
  HandleRequest(WRITE, "delAlarmsRange", params, response);
}
void AnalyzerRemote::DelAlarm(const json &params, Response &response) {
  HandleRequest(WRITE, "delAlarm", params, response);
}
void AnalyzerRemote::GetRates(const json &params, Response &response) {
  HandleRequest(READ, "getRates", params, response);
}
void AnalyzerRemote::AddRate(const json &params, Response &response) {
  HandleRequest(CONFIG, "addRate", params, response);
}
void AnalyzerRemote::GetRate(const json &params, Response &response) {
  HandleRequest(READ, "getRate", params, response);
}
void AnalyzerRemote::SetRate(const json &params, Response &response) {
  HandleRequest(CONFIG, "setRate", params, response);
}
void AnalyzerRemote::DelRate(const json &params, Response &response) {
  HandleRequest(CONFIG, "delRate", params, response);
}
bool AnalyzerRemote::HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) {
//  uint8_t Pending = len;

  return false;

}
//----------------------------------------------------------------------------------------------------------------------