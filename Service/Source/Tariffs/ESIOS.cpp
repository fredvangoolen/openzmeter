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
vector<ESIOS::Variable_t> ESIOS::pVariables = {
  {  773, 8741, "hour", "v773",  "(773) Pagos por capacidad tarifa seis periodos" },
  {  792, 8741, "hour", "v792",  "(792) Precio medio horario componente mercado diario contratación libre" },
  {  793, 8741, "hour", "v793",  "(793) Precio medio horario componente restricciones PBF contratación libre" },
  {  794, 8741, "hour", "v794",  "(794) Precio medio horario componente restricciones tiempo real contratación libre" },
  {  797, 8741, "hour", "v797",  "(797) Precio medio horario componente reserva de potencia adicional a subir contratación libre" },
  {  798, 8741, "hour", "v798",  "(798) Precio medio horario componente banda secundaria contratación libre" },
  {  799, 8741, "hour", "v799",  "(799) Precio medio horario componente desvíos medidos contratación libre" },
  {  800, 8741, "hour", "v800",  "(800) Precio medio horario componente saldo de desvíos contratación libre" },
  {  802, 8741, "hour", "v802",  "(802) Precio medio horario componente saldo P.O.14.6 contratación libre" },
  {  803, 8741, "hour", "v803",  "(803) Precio medio horario componente fallo nominación UPG contratación libre" },
  { 1277, 8741, "hour", "v1277", "(1277) Precio medio horario componente servicio interrumpibilidad " },
  { 1286, 8741, "hour", "v1286", "(1286) Precio medio horario componente control factor potencia " },
  { 1368, 8741, "hour", "v1368", "(1368) Precio medio horario componente incumplimiento energía de balance " },
};
// ---------------------------------------------------------------------------------------------------------------------
ESIOS::ESIOS() {
  Database DB;
  for(uint16_t V = 0; V < pVariables.size(); V++) DB.ExecSentenceNoResult("ALTER TABLE esios ADD COLUMN IF NOT EXISTS " + DB.BindID(pVariables[V].VarName) + " real NULL");
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
    uint64_t To    = pSyncDate + 863999999;
    pthread_mutex_unlock(&pMutex);
    LOGFile::Debug("ESIOS request start (Up to %" PRIu64 ")\n", To);
    bool     Fail   = false;
    for(uint16_t V = 0; (Fail == false) && (V < pVariables.size()); V++) Fail = !FetchIndicator(Client, Token, pVariables[V], To);
    if(Fail == true) {
      usleep(1000000);
    } else {
      LOGFile::Debug("ESIOS request finished\n");
      pthread_mutex_lock(&pMutex);
      if(pSyncDate < (Tools::GetTime64() + 864000000)) pSyncDate = pSyncDate + 864000000;
      pthread_mutex_unlock(&pMutex);
      for(uint32_t Loop = 0; (pTerminate == false) && (To > Tools::GetTime64()) && (Loop < 6000); Loop++) usleep(100000);
    }
  }
  return NULL;
}
bool ESIOS::FetchIndicator(SSLClient &client, const string &token, const Variable_t &var, uint64_t to) {
  Database DB;
  time_t   Time;
  tm       Timestamp;
  char     DateBuffFrom[64], DateBuffTo[64], URL[256];
  Headers  Header = { {"Accept", "application/json"}, {"Host", "api.esios.ree.es"}, {"Authorization", "Token token=\"" + token + "\""}, {"Content-Type", "application/x-www-form-urlencoded" } };
  DB.ExecSentenceAsyncResult("SELECT GREATEST(COALESCE(MAX(timestamp) + 1, 1262304000000), 1262304000000) AS min FROM esios WHERE " + DB.BindID(var.VarName) +" IS NOT NULL");
  if(DB.FecthRow() == false) return false;
  Time = DB.ResultUInt64("min") / 1000;
  if((uint64_t)Time > (to / 1000)) return true;
  strftime(DateBuffFrom, sizeof(DateBuffFrom), "%Y-%m-%dT%H:%M:%SZ", gmtime_r(&Time, &Timestamp));
  Time =  to / 1000;
  strftime(DateBuffTo, sizeof(DateBuffTo), "%Y-%m-%dT%H:%M:%SZ", gmtime_r(&Time, &Timestamp));
  snprintf(URL, sizeof(URL), "/indicators/%u?locale=es&time_trunc=%s&geo_ids=%u&start_date=%s&end_date=%s", var.IndicatorNum, var.TimeTrunc.c_str(), var.GeoID, DateBuffFrom, DateBuffTo);
  auto Response = client.Get(URL, Header);
  if((Response == nullptr) || (Response->status != 200)) return false;
  json Params = json::parse(Response->body, NULL, false);
  if((Params.is_discarded() == true) || (Params["indicator"].is_object() == false) || (Params["indicator"]["values"].is_array() == false) || (Params["indicator"]["name"].is_string() == false)) return false;
  if(Params["indicator"]["values"].size() == 0) return true;
  string Query = "INSERT INTO esios(timestamp, " + DB.BindID(var.VarName) + ") VALUES ";
  for(auto &Item : Params["indicator"]["values"]) {
    if((Item.is_object() == false) || (Item["value"].is_number() == false) || (Item["tz_time"].is_string() == false) || (strptime(Item["tz_time"].get<string>().c_str(), "%Y-%m-%dT%H:%M:%S%z", &Timestamp) != NULL)) return false;
    Query += "(" + DB.Bind(((uint64_t)timegm(&Timestamp)) * 1000 + 3599999) + ", " + DB.Bind(Item["value"].get<float>()) + "), ";
  }
  Query.resize(Query.size() - 2);
  Query += " ON CONFLICT(timestamp) DO UPDATE SET " + DB.BindID(var.VarName) + " = EXCLUDED." + DB.BindID(var.VarName);
  return DB.ExecSentenceNoResult(Query);
}
//----------------------------------------------------------------------------------------------------------------------
bool ESIOS_PriceModule::pRegistered = ESIOS_PriceModule::Register();
//----------------------------------------------------------------------------------------------------------------------
bool ESIOS_PriceModule::Register() {
  PriceModule::pModules.insert( { "ESP_ESIOS", { CreateInstance, GetVarsInfo } } );
  return true;
}
PriceModule *ESIOS_PriceModule::CreateInstance() {
  return new ESIOS_PriceModule();
}
ESIOS_PriceModule::ESIOS_PriceModule() {
  for(uint16_t V = 0; V < ESIOS::pVariables.size(); V++) pQueryColumns += ESIOS::pVariables[V].VarName + ", ";
  pQueryColumns += "timestamp";
}
ESIOS_PriceModule::~ESIOS_PriceModule() {
}
void ESIOS_PriceModule::PrepareRange(const uint64_t from, const uint64_t to) {
  pDB.ExecSentenceAsyncResult("SELECT " + pQueryColumns + " FROM esios WHERE timestamp >= " + pDB.Bind(from) + " AND timestamp <=" + pDB.Bind(to) + " ORDER BY timestamp ASC");
  pHaveData = pDB.FecthRow();
}
void ESIOS_PriceModule::GetParams(const uint64_t sampletime, map<string, float> &vars) {
  while((pHaveData == true) && ((pDB.ResultUInt64("timestamp") / 3600000) < (sampletime / 3600000))) pHaveData = pDB.FecthRow();
  if((pHaveData == true) && ((pDB.ResultUInt64("timestamp") / 3600000) == (sampletime / 3600000))) {
    for(uint16_t V = 0; V < ESIOS::pVariables.size(); V++) vars[ESIOS::pVariables[V].VarName] = pDB.ResultFloat(ESIOS::pVariables[V].VarName);
  } else {
    for(uint16_t V = 0; V < ESIOS::pVariables.size(); V++) vars[ESIOS::pVariables[V].VarName] = 0.0;
  }

//      printf("ESIOS - ");
//      for(auto Var : vars) printf("%s: %.10f, ", Var.first.c_str(), Var.second);

}
json ESIOS_PriceModule::GetVarsInfo() {
  json Tmp = json::array();
  for(uint16_t V = 0; V < ESIOS::pVariables.size(); V++) {
    json Var = json::object();
    Var["Name"] = ESIOS::pVariables[V].VarName;
    Var["Description"] = ESIOS::pVariables[V].Description;
    Tmp.push_back(Var);
  }
  return Tmp;
}