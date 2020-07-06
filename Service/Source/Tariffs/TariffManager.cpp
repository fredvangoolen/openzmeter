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
#include "TariffManager.h"
#include "Tariff.h"
// ---------------------------------------------------------------------------------------------------------------------
TariffManager::TariffManager() {
  HTTPServer::RegisterCallback("getTariffs",     GetTariffsCallback);
  HTTPServer::RegisterCallback("getTariff",      GetTariffCallback);
  HTTPServer::RegisterCallback("setTariff",      SetTariffCallback);
  HTTPServer::RegisterCallback("delTariff",      DelTariffCallback);
  HTTPServer::RegisterCallback("addTariff",      AddTariffCallback);
}
TariffManager::~TariffManager() {
  HTTPServer::UnregisterCallback("getTariffs");
  HTTPServer::UnregisterCallback("getTariff");
  HTTPServer::UnregisterCallback("setTariff");
  HTTPServer::UnregisterCallback("delTariff");
  HTTPServer::UnregisterCallback("addTariff");
}
void TariffManager::GetTariffsCallback(const json &params, Response &response) {
  Database DB;
  string   UserName = params["UserName"];
  DB.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('Name', name, 'UserName', username))), '[]') as result FROM tariffs" + ((UserName == "admin") ? "" : " WHERE username IN ('admin', " + DB.Bind(UserName) + ")"));
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void TariffManager::GetTariffCallback(const json &params, Response &response) {
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is required", "text/plain", HTTP_BAD_REQUEST);
  Database DB;
  string Name = params["Name"];
  string UserName = params["UserName"];
  DB.ExecSentenceAsyncResult("SELECT json_build_object('Name', name, 'UserName', username, 'Template', template) as result FROM tariffs WHERE name = " + DB.Bind(Name) + ((UserName == "admin") ? "" : " AND username = " + DB.Bind(UserName)));
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void TariffManager::DelTariffCallback(const json &params, Response &response) {
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is required", "text/plain", HTTP_BAD_REQUEST);
  Database DB;
  string Name = params["Name"];
  string UserName = params["UserName"];
  DB.ExecSentenceAsyncResult("DELETE FROM tariffs WHERE name = " + DB.Bind(Name) + ((UserName == "admin") ? "" : " AND username = " + DB.Bind(UserName)) + " RETURNING name");
  return HTTPServer::FillResponse(response, "", "text/plain", (!DB.FecthRow()) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
void TariffManager::SetTariffCallback(const json &params, Response &response) {
  if((params.contains("Template") == false) || (params["Template"].is_object() == false)) return HTTPServer::FillResponse(response, "'Template' param is required", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is required", "text/plain", HTTP_BAD_REQUEST);
  json Conf = Rates::CheckTariffConfig(params["Template"]);
  if(Conf.is_string()) return HTTPServer::FillResponse(response, Conf, "text/plain", HTTP_BAD_REQUEST);
  string   UserName = params["UserName"];
  string   Name     = params["Name"];
  Database DB;
  DB.ExecSentenceAsyncResult("UPDATE tariffs SET name = " + DB.Bind(Name) + ", template = " + DB.Bind(Conf.dump()) + " WHERE name = " + DB.Bind(Name) + ((UserName != "admin") ? " AND username = " + DB.Bind(UserName) : "") + " RETURNING name");
  return HTTPServer::FillResponse(response, "", "text/plain", (!DB.FecthRow()) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
void TariffManager::AddTariffCallback(const json &params, Response &response) {
  if((params.contains("Template") == false) || (params["Template"].is_object() == false)) return HTTPServer::FillResponse(response, "'Template' param is required", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is required", "text/plain", HTTP_BAD_REQUEST);
  json Conf = Rates::CheckTariffConfig(params["Template"]);
  if(Conf.is_string()) return HTTPServer::FillResponse(response, Conf, "text/plain", HTTP_BAD_REQUEST);
  string   UserName = params["UserName"];
  string   Name     = params["Name"];
  Database DB;
  DB.ExecSentenceAsyncResult("INSERT INTO tariffs(name, username, template) VALUES(" + DB.Bind(Name) + ", " + DB.Bind(UserName) + ", " + DB.Bind(Conf.dump()) + ") RETURNING name");
  return HTTPServer::FillResponse(response, "", "text/plain", (!DB.FecthRow()) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
// ---------------------------------------------------------------------------------------------------------------------