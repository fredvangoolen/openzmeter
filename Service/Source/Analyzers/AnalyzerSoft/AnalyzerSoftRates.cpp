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
#include "RatesEval.h"
#include "AnalyzerSoftRates.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftRates::AnalyzerSoftRates(const string &serial) {
  pSerial = serial;
  pLock   = PTHREAD_RWLOCK_INITIALIZER;
  pRates.resize(10);
}
AnalyzerSoftRates::~AnalyzerSoftRates() {
}
json AnalyzerSoftRates::CheckConfig(const json &config, const bool except) {
  json ConfigOUT = json::array();
  if(config.is_array() == false) return ConfigOUT;
  for(auto &Rate : config) {
    try {
      ConfigOUT.push_back(Rate::CheckConfig(Rate));
    } catch(const string &err) {
      if(except == true) throw err;
    }
  }
  return ConfigOUT;
}
void AnalyzerSoftRates::Config(const string &timezone, const json &rates) {
  pthread_rwlock_wrlock(&pLock);
  load_time_zone(timezone, &pTimeZone);
  for(uint8_t n = 0; n < 10; n++) pRates[n].clear();
  for(auto &Rate : rates) pRates[(uint8_t)Rate["Group"]].emplace_back(Rate, pTimeZone);
  for(uint8_t n = 0; n < 10; n++) sort(pRates[n].begin(), pRates[n].end(), [](Rate &a, Rate &b) { return a.StartDate() < b.StartDate(); });
  pthread_rwlock_unlock(&pLock);
}
json AnalyzerSoftRates::GetConfig() {
  json Result = json::array();
  pthread_rwlock_rdlock(&pLock);
  for(auto &Group : pRates) {
    for(auto &Rate : Group) Result.push_back(Rate.GetConfig());
  }
  pthread_rwlock_unlock(&pLock);
  return Result;
}
void AnalyzerSoftRates::GetCosts(const json &params, Response &response) {
  if((params.contains("To") == false) || (params["To"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(params["From"] > params["To"]) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Group") == false) || (params["Group"].is_number_integer() == false) || (params["Group"] < 0) || (params["Group"] > 10)) return HTTPServer::FillResponse(response, "'Group' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  uint8_t  Group  = params["Group"];
  uint16_t Series = RatesEval::SERIE_TIMESTAMP;
  for(auto &Serie : params["Series"]) {
    if(Serie.is_string() == false) {
      return HTTPServer::FillResponse(response, "'Series' must be array of strings", "text/plain", HTTP_BAD_REQUEST);
    } else if(Serie == "Active_Power") {
      Series |= RatesEval::SERIE_ACTIVE_POWER;
    } else if(Serie == "Active_Energy") {
      Series |= RatesEval::SERIE_ACTIVE_ENERGY;
    } else if(Serie == "Reactive_Power") {
      Series |= RatesEval::SERIE_REACTIVE_POWER;
    } else if(Serie == "Reactive_Energy") {
      Series |= RatesEval::SERIE_REACTIVE_ENERGY;
    } else if(Serie == "Cost") {
      Series |= RatesEval::SERIE_COST;
    } else if(Serie == "Power_Cost") {
      Series |= RatesEval::SERIE_POWER_COST;
    } else if(Serie == "Energy_Cost") {
      Series |= RatesEval::SERIE_ENERGY_COST;
    } else if(Serie == "Period") {
      Series |= RatesEval::SERIE_PERIOD;
    } else if(Serie == "Color") {
      Series |= RatesEval::SERIE_COLOR;
    } else if(Serie == "Contracted") {
      Series |= RatesEval::SERIE_CONTRACTED;
    } else if(Serie == "Bill") {
      Series |= RatesEval::SERIE_BILL;
    } else {
      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  auto Func = [&, From, To, Group, Series](uint64_t offset, DataSink &sink) {
    Database DB(pSerial);
    auto GetSample = [&](const uint64_t &currenttime, float &active, float &reactive) -> uint64_t {
      if(DB.FecthRow() == true) {
        active     = DB.ResultFloat("active");
        reactive   = DB.ResultFloat("reactive");
        return DB.ResultUInt64("sampletime");
      } else {
        DB.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive FROM aggreg_15m WHERE sampletime >= " + DB.Bind(currenttime) + " ORDER BY sampletime ASC LIMIT 1000");
        if(DB.FecthRow() == true) {
          active     = DB.ResultFloat("active");
          reactive   = DB.ResultFloat("reactive");
          return DB.ResultUInt64("sampletime");
        }
      }
      return UINT64_MAX;
    };
    pthread_rwlock_rdlock(&pLock);
    RatesEval Eval(pTimeZone, From, To, pRates[Group], GetSample, Series);
    pthread_rwlock_unlock(&pLock);
    sink.os << "[\n  " << Eval.Headers().dump();
    json Result = Eval.FetchRow();
    while((Result != nullptr) && sink.is_writable()) {
      sink.os << ",\n  " + Result.dump();
      Result = Eval.FetchRow();
    }
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerSoftRates::GetBills(const json &params, Response &response) {
  if((params.contains("To") == false) || (params["To"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(params["From"] > params["To"]) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Group") == false) || (params["Group"].is_number_integer() == false) || (params["Group"] < 0) || (params["Group"] > 10)) return HTTPServer::FillResponse(response, "'Group' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t From  = params["From"];
  uint64_t To    = params["To"];
  uint8_t  Group = params["Group"];
  auto Func = [&, From, To, Group](uint64_t offset, DataSink &sink) {
    Database DB(pSerial);
    auto GetSample = [&](const uint64_t &currenttime, float &active, float &reactive) -> uint64_t {
      if(DB.FecthRow() == true) {
        active     = DB.ResultFloat("active");
        reactive   = DB.ResultFloat("reactive");
        return DB.ResultUInt64("sampletime");
      } else {
        DB.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive FROM aggreg_15m WHERE sampletime >= " + DB.Bind(currenttime) + " ORDER BY sampletime ASC LIMIT 1000");
        if(DB.FecthRow() == true) {
          active     = DB.ResultFloat("active");
          reactive   = DB.ResultFloat("reactive");
          return DB.ResultUInt64("sampletime");
        }
      }
      return UINT64_MAX;
    };
    uint32_t Items = 0;
    pthread_rwlock_rdlock(&pLock);
    RatesEval Eval(pTimeZone, From, To, pRates[Group], GetSample);
    pthread_rwlock_unlock(&pLock);
    sink.os << "[\n  ";
    json Result = Eval.FetchRow();
    while((Result != nullptr) && sink.is_writable()) {
      sink.os << ((Items++ != 0) ? ",\n  " + Result.dump() : Result.dump());
      Result = Eval.FetchRow();
    }
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);
}
// ---------------------------------------------------------------------------------------------------------------------