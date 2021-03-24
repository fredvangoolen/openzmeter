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
#include "AnalyzerRates.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerRates::AnalyzerRates(const string &serial) : pSerial(serial) {
  pLock = PTHREAD_RWLOCK_INITIALIZER;
  pRates.resize(10);
}
AnalyzerRates::~AnalyzerRates() {
}
json AnalyzerRates::CheckConfig(const json &config, const bool except) {
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
void AnalyzerRates::Config(const string &timezone, const json &rates) {
  pthread_rwlock_wrlock(&pLock);
  load_time_zone(timezone, &pTimeZone);
  for(uint8_t n = 0; n < 10; n++) pRates[n].clear();
  for(auto &Rate : rates) pRates[(uint8_t)Rate["Group"]].emplace_back(Rate, pTimeZone);
  for(uint8_t n = 0; n < 10; n++) sort(pRates[n].begin(), pRates[n].end(), [](Rate &a, Rate &b) { return a.StartDate() < b.StartDate(); });
  pthread_rwlock_unlock(&pLock);
}
void AnalyzerRates::GetCosts(HTTPRequest &req) {
  if(!req.CheckParamNumber("To") || !req.CheckParamNumber("From") || !req.CheckParamNumber("Group") || !req.CheckParamSet("Series", {"Active_Power", "Active_Energy", "Reactive_Power", "Reactive_Energy", "Cost", "Power_Cost", "Energy_Cost", "Period", "Color", "Contracted", "Bill"})) return;
  uint64_t       From    = req.Param("From");
  uint64_t       To      = req.Param("To");
  uint8_t        Group   = req.Param("Group");
  vector<string> Headers = {"Time"};
  for(auto &Serie : req.Param("Series")) Headers.push_back(Serie);
  auto Func = [&, From, To, Group, Headers](function<bool(const json &content)> Write) {
    Database DB(pSerial);
    auto GetSample = [&](const uint64_t &currenttime, float &active, float &reactive) -> uint64_t {
      if(DB.FecthRow() == true) {
        active     = DB.ResultFloat(1);
        reactive   = DB.ResultFloat(2);
        return DB.ResultUInt64(0);
      } else {
        DB.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s), (SELECT SUM(s) FROM UNNEST(reactive_power) s) FROM aggreg_15m WHERE sampletime >= " + DB.Bind(currenttime) + " ORDER BY sampletime ASC LIMIT 100");
        if(DB.FecthRow() == true) {
          active     = DB.ResultFloat(1);
          reactive   = DB.ResultFloat(2);
          return DB.ResultUInt64(0);
        }
      }
      return UINT64_MAX;
    };
    pthread_rwlock_rdlock(&pLock);
    RatesEval Eval(pTimeZone, From, To, pRates[Group], GetSample, Headers);
    pthread_rwlock_unlock(&pLock);
    Write(Headers);
    while(true) {
      json Result = Eval.FetchRow();
      if((Result == nullptr) || (Write(Result) == false)) break;
    }
  };
  req.SuccessStreamArray(Func);
}
void AnalyzerRates::GetBills(HTTPRequest &req) {
  if(!req.CheckParamNumber("To") || !req.CheckParamNumber("From") || !req.CheckParamNumber("Group")) return;
  uint64_t From  = req.Param("From");
  uint64_t To    = req.Param("To");
  uint8_t  Group = req.Param("Group");
  if((Group < 0) || (Group > 10)) return req.Error("Invalid 'Group' param range", HTTP_BAD_REQUEST);
  auto Func = [&, From, To, Group](function<bool(const json &content)> Write) {
    Database DB(pSerial);
    auto GetSample = [&](const uint64_t &currenttime, float &active, float &reactive) -> uint64_t {
      if(DB.FecthRow() == true) {
        active   = DB.ResultFloat(1);
        reactive = DB.ResultFloat(2);
        return DB.ResultUInt64(0);
      } else {
        DB.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s), (SELECT SUM(s) FROM UNNEST(reactive_power) s) FROM aggreg_15m WHERE sampletime >= " + DB.Bind(currenttime) + " ORDER BY sampletime ASC LIMIT 1000");
        if(DB.FecthRow() == true) {
          active   = DB.ResultFloat(1);
          reactive = DB.ResultFloat(2);
          return DB.ResultUInt64(0);
        }
      }
      return UINT64_MAX;
    };
    pthread_rwlock_rdlock(&pLock);
    RatesEval Eval(pTimeZone, From, To, pRates[Group], GetSample);
    pthread_rwlock_unlock(&pLock);
    while(true) {
     json Result = Eval.FetchRow();
     if((Result == nullptr) || (Write(Result) == false)) break;
    }
  };
  req.SuccessStreamArray(Func);
}
// ---------------------------------------------------------------------------------------------------------------------