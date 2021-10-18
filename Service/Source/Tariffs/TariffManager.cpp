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
#include "HTTPServer.h"
#include "TariffManager.h"
#include "Tariff.h"
#include "RatesEval.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
TariffManager::TariffManager() {
  LogFile::Info("Starting TariffManager...");
  HTTPServer::RegisterDoc("Tariffs", "API/Tariffs/Tariffs.json");
  HTTPServer::RegisterCallback("getTariffs", GetTariffsCallback, "API/Tariffs/GetTariffs.json");
  HTTPServer::RegisterCallback("getTariff",  GetTariffCallback,  "API/Tariffs/GetTariff.json");
  HTTPServer::RegisterCallback("setTariff",  SetTariffCallback,  "API/Tariffs/SetTariff.json");
  HTTPServer::RegisterCallback("delTariff",  DelTariffCallback,  "API/Tariffs/DelTariff.json");
  HTTPServer::RegisterCallback("addTariff",  AddTariffCallback,  "API/Tariffs/AddTariff.json");
//  HTTPServer::RegisterCallback("evalRates",  EvalRatesCallback);
  LogFile::Info("TariffManager started");
}
TariffManager::~TariffManager() {
  LogFile::Info("Stopping TariffManager...");
  HTTPServer::UnregisterCallback("getTariffs");
  HTTPServer::UnregisterCallback("getTariff");
  HTTPServer::UnregisterCallback("setTariff");
  HTTPServer::UnregisterCallback("delTariff");
  HTTPServer::UnregisterCallback("addTariff");
//  HTTPServer::UnregisterCallback("evalRates");
  HTTPServer::UnregisterDoc("Tariffs");
  LogFile::Info("TariffManager stoped");
}
void TariffManager::GetTariffsCallback(HTTPRequest &req) {
  string UserName = req.UserName();
  auto Func = [UserName](function<bool(const json &content)> Write) {
    json Header = {"Name", "User"};
    Write(Header);
    Database DB;
    DB.ExecSentenceAsyncResult("SELECT json_build_array(name, username) FROM tariffs" + ((UserName == "admin") ? "" : " WHERE username IN ('admin', " + DB.Bind(UserName) + ") ORDER BY name ASC"));
    while(DB.FecthRow()) {
      if(Write(DB.ResultJSON(0)) == false) return;
    }
  };
  req.SuccessStreamArray(Func);
}
void TariffManager::GetTariffCallback(HTTPRequest &req) {
  if(!req.CheckParamString("Name")) return;
  Database DB;
  string Name = req.Param("Name");
  if(!DB.ExecSentenceAsyncResult("SELECT json_build_object('Name', name, 'User', username, 'Template', template) as result FROM tariffs WHERE name = " + DB.Bind(Name) + ((req.UserName() == "admin") ? "" : " AND username = " + DB.Bind(req.UserName()))) || !DB.FecthRow()) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(DB.ResultJSON(0));
}
void TariffManager::DelTariffCallback(HTTPRequest &req) {
  if(!req.CheckParamString("Name")) return;
  Database DB;
  string Name = req.Param("Name");
  if(!DB.ExecSentenceAsyncResult("DELETE FROM tariffs WHERE name = " + DB.Bind(Name) + ((req.UserName() == "admin") ? "" : " AND username = " + DB.Bind(req.UserName())) + " RETURNING name")) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(json::object());
}
void TariffManager::SetTariffCallback(HTTPRequest &req) {
  if(!req.CheckParamObject("Template") || !req.CheckParamString("Name")) return;
  try {
    json Conf = Tariff::CheckConfig(req.Param("Template"));
    if(Conf.is_string()) return req.Error(Conf, HTTP_BAD_REQUEST);
    string   Name = req.Param("Name");
    Database DB;
    if((DB.ExecSentenceAsyncResult("UPDATE tariffs SET name = " + DB.Bind(Name) + ", template = " + DB.Bind(Conf.dump()) + " WHERE name = " + DB.Bind(Name) + ((req.UserName() != "admin") ? " AND username = " + DB.Bind(req.UserName()) : "") + " RETURNING name") == false) || (DB.FecthRow() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    req.Success(json::object());
  } catch(const string &err) {
    req.Error(err, HTTP_BAD_REQUEST);
  }
}
void TariffManager::AddTariffCallback(HTTPRequest &req) {
  if(!req.CheckParamObject("Template") || !req.CheckParamString("Name")) return;
  try {
    json Conf = Tariff::CheckConfig(req.Param("Template"));
    if(Conf.is_string()) return req.Error(Conf, HTTP_BAD_REQUEST);
    string   Name = req.Param("Name");
    Database DB;
    if(!DB.ExecSentenceAsyncResult("INSERT INTO tariffs(name, username, template) VALUES(" + DB.Bind(Name) + ", " + DB.Bind(req.UserName()) + ", " + DB.Bind(Conf.dump()) + ") RETURNING name") || !DB.FecthRow()) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    req.Success(json::object());
  } catch(const string &err) {
    req.Error(err, HTTP_BAD_REQUEST);
  }
}
//void TariffManager::EvalRatesCallback(const json &params, Response &response) {
//  if((params.contains("Rates") == false) || (params["Rates"].is_array() == false)) return HTTPServer::FillResponse(response, "'Rates' param is required", "text/plain", HTTP_BAD_REQUEST);
//  if((params.contains("TimeZone") == false) || (params["TimeZone"].is_string() == false)) return HTTPServer::FillResponse(response, "'TimeZone' param is required", "text/plain", HTTP_BAD_REQUEST);
//  if((params.contains("Samples") == false) || (params["Samples"].is_array() == false)) return HTTPServer::FillResponse(response, "'Samples' param is required", "text/plain", HTTP_BAD_REQUEST);
//  if((params.contains("To") == false) || (params["To"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'To' param is required", "text/plain", HTTP_BAD_REQUEST);
//  if((params.contains("From") == false) || (params["From"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'From' param is required", "text/plain", HTTP_BAD_REQUEST);
//  if(params["From"] > params["To"]) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
//  time_zone TimeZone;
//  uint64_t  From     = params["From"];
//  uint64_t  To       = params["To"];
//  json      Response = json::array();
//  cctz::load_time_zone(params["TimeZone"], &TimeZone);
  //RatesEvalProvider Provider(TimeZone, params["Rates"], params["Samples"]);
//  RatesEval Eval(TimeZone, From, To, Provider);
//  json Result = Eval.FetchRow();
//  while(Result != nullptr) {
//    Response.push_back(Result);
//    Result = Eval.FetchRow();
//  }
//  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
//}
// ---------------------------------------------------------------------------------------------------------------------