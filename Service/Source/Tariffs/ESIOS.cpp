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
#include "cctz/time_zone.h"
#include "Database.h"
#include "ESIOS.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace cctz;
// ---------------------------------------------------------------------------------------------------------------------
uint64_t                  ESIOS::pSyncDate    = 1262304000000;
string                    ESIOS::pToken       = "";
pthread_t                 ESIOS::pThread      = 0;
pthread_mutex_t           ESIOS::pMutex       = PTHREAD_MUTEX_INITIALIZER;
volatile bool             ESIOS::pTerminate   = false;
// ---------------------------------------------------------------------------------------------------------------------
ESIOS::ESIOS() {
  HTTPServer::RegisterCallback("getESIOS", GetESIOSCallback);
  HTTPServer::RegisterCallback("setESIOS", SetESIOSCallback);
  Configure(CheckConfig(ConfigManager::ReadConfig("/ESIOS")));
}
ESIOS::~ESIOS() {
  pthread_mutex_lock(&pMutex);
  pTerminate = true;
  pthread_mutex_unlock(&pMutex);
  if(pThread != 0) pthread_join(pThread, NULL);
}
json ESIOS::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Token"] = "";
  ConfigOUT["Enabled"] = false;
  if(config.is_object() == true) {
    if(config.contains("Token") && config["Token"].is_string()) ConfigOUT["Token"] = config["Token"];
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
  }
  return ConfigOUT;
}
void ESIOS::Configure(const json &config) {
  ConfigManager::WriteConfig("/ESIOS", config);
  bool Enable = config["Enabled"];
  pthread_mutex_lock(&pMutex);
  pToken    = config["Token"];
  if((Enable == true) && (pThread == 0)) {
    pTerminate = false;
    if(pthread_create(&pThread, NULL, Thread_Func, NULL) != 0) {
      pThread = 0;
      LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    } else {
      pthread_setname_np(pThread, "ESIOSClient");
    }
  } else if((Enable == false) && (pThread != 0)) {
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
  }
  pthread_mutex_unlock(&pMutex);
}
void ESIOS::GetESIOSCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigManager::ReadConfig("/ESIOS");
  pthread_mutex_lock(&pMutex);
  Config["LastDate"] = pSyncDate;
  pthread_mutex_unlock(&pMutex);
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void ESIOS::SetESIOSCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Enabled") == false) || (params["Enabled"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Enabled' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Token") == false) || (params["Token"].is_string() == false)) return HTTPServer::FillResponse(response, "'Token' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Configure(CheckConfig(params));
  HTTPServer::FillResponse(response, "", "application/json", HTTP_OK);
}
void *ESIOS::Thread_Func(void *params) {
  ESIOS    *This = (ESIOS*)params;
  SSLClient Client("api.esios.ree.es", 443);
  Client.set_keep_alive_max_count(100);
  Client.set_read_timeout(20, 0);
  Client.set_timeout_sec(30);
  Client.set_compress(true);
  while(pTerminate == false) {
    pthread_mutex_lock(&pMutex);
    string   Token = This->pToken;
    uint64_t To = pSyncDate + 863999999;
    pthread_mutex_unlock(&pMutex);
    LOGFile::Debug("ESIOS request start (Up to %" PRIu64 ")\n", To);
    if(FetchIndicator(&Client, Token, "773", "ESIOS_773_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    if(FetchIndicator(&Client, Token, "792", "ESIOS_792_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    if(FetchIndicator(&Client, Token, "793", "ESIOS_793_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    if(FetchIndicator(&Client, Token, "794", "ESIOS_794_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    if(FetchIndicator(&Client, Token, "797", "ESIOS_797_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    if(FetchIndicator(&Client, Token, "798", "ESIOS_798_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    if(FetchIndicator(&Client, Token, "799", "ESIOS_799_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    if(FetchIndicator(&Client, Token, "800", "ESIOS_800_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    if(FetchIndicator(&Client, Token, "802", "ESIOS_802_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    if(FetchIndicator(&Client, Token, "803", "ESIOS_803_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    if(FetchIndicator(&Client, Token, "1277", "ESIOS_1277_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    if(FetchIndicator(&Client, Token, "1286", "ESIOS_1286_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    if(FetchIndicator(&Client, Token, "1368", "ESIOS_1368_Precio_Hora_Peninsula", "hour", "8741", To) == false) { usleep(1000000); continue; }
    LOGFile::Debug("ESIOS request finished\n");
    pthread_mutex_lock(&pMutex);
    pSyncDate = pSyncDate + 864000000;
    pthread_mutex_unlock(&pMutex);
    for(uint32_t Loop = 0; (pTerminate == false) && (To > Tools::GetTime64()) && (Loop < 6000); Loop++) usleep(100000);
  }
  return NULL;
}
bool ESIOS::FetchIndicator(SSLClient *client, const string &token, const string &indicator, const string &shortName, const string &time_trunc, const string &geo_id, uint64_t to) {
  Database DB;
  time_t   Time;
  tm       Timestamp;
  char     DateBuffFrom[64], DateBuffTo[64], URL[256];
  Headers  Header = { {"Accept", "application/json"}, {"Host", "api.esios.ree.es"}, {"Authorization", "Token token=\"" + token + "\""}, {"Content-Type", "application/x-www-form-urlencoded" } };
  DB.ExecSentenceAsyncResult("SELECT GREATEST(COALESCE(MAX(timestamp) + 1, 1262304000000), 1262304000000) AS min FROM esios WHERE keyvalue = (SELECT keyvalue FROM esios_index WHERE keyname = " + DB.Bind(shortName) + ")");
  if(DB.FecthRow()) Time = DB.ResultUInt64("min") / 1000;
  if((uint64_t)Time > (to / 1000)) return true;
  strftime(DateBuffFrom, sizeof(DateBuffFrom), "%Y-%m-%dT%H:%M:%SZ", gmtime_r(&Time, &Timestamp));
  Time =  to / 1000;
  strftime(DateBuffTo, sizeof(DateBuffTo), "%Y-%m-%dT%H:%M:%SZ", gmtime_r(&Time, &Timestamp));
  snprintf(URL, sizeof(URL), "/indicators/%s?locale=es&time_trunc=%s&geo_ids=%s&start_date=%s&end_date=%s", indicator.c_str(), time_trunc.c_str(), geo_id.c_str(), DateBuffFrom, DateBuffTo);
  auto Response = client->Get(URL, Header);
  if((Response != nullptr) && (Response->status == 200)) {
    json Params = json::parse(Response->body, NULL, false);
    if((Params.is_discarded() == false) && (Params["indicator"].is_object() == true) && (Params["indicator"]["values"].is_array() == true) && (Params["indicator"]["name"].is_string() == true)) {
      if(Params["indicator"]["values"].size() == 0) return true;
      string Query = "WITH t1 AS (SELECT " + DB.Bind(shortName) + "::text AS keyname),"
                     "     t2 AS (INSERT INTO esios_index(keyname, shortname) SELECT keyname, " + DB.Bind(Params["indicator"]["name"].get<string>()) + " FROM t1 ON CONFLICT (keyname) DO NOTHING RETURNING keyvalue),"
                     "     t3 AS (SELECT COALESCE((SELECT keyvalue FROM t2), (SELECT keyvalue FROM esios_index, t1 WHERE t1.keyname = esios_index.keyname)) AS keyvalue) "
                    "INSERT INTO esios(keyvalue, timestamp, value) SELECT keyvalue, timestamp, value FROM (VALUES ";
      for(auto &Item : Params["indicator"]["values"]) {
        if((Item.is_object() == false) || (Item["value"].is_number() == false) || (Item["tz_time"].is_string() == false) || (strptime(Item["tz_time"].get<string>().c_str(), "%Y-%m-%dT%H:%M:%S%z", &Timestamp) != NULL)) return false;
        Query += "(" + DB.Bind(((uint64_t)timegm(&Timestamp)) * 1000 + 3599999) + ", " + DB.Bind(Item["value"].get<float>()) + "), ";
      }
      Query.resize(Query.size() - 2);
      Query += ") AS t(timestamp, value), t3 ON CONFLICT(keyvalue, timestamp) DO NOTHING";
      return DB.ExecSentenceNoResult(Query);
    }
  }
  return false;
}