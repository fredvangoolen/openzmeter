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
#include "AnalyzerSoftRates.h"
#include "HTTPChunkedWriter.h"
#include "Rate.h"
#include "AnalyzerSoftRatesEval.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftRates::AnalyzerSoftRates(const string &serial) {
  pSerial = serial;
}
AnalyzerSoftRates::~AnalyzerSoftRates() {
}
void AnalyzerSoftRates::Config(const time_zone &tz) {
  pTimeZone = tz;
}
void AnalyzerSoftRates::GetRates(const json &params, Response &response) const {
  auto WriteFunc = [=](HTTPChunkedWriter *writer) -> bool {
    uint32_t Items = 0;
    writer->Write("[\n  ");
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_object('RateID', id, 'Name', name, 'Group', ordering, 'StartDate', to_char(startdate, 'dd/mm/yyyy'), 'EndDate', to_char(enddate, 'dd/mm/yyyy')) AS result FROM rates");
    while(DB.FecthRow()) writer->Write(((Items++ != 0) ? ",\n  " : "") + DB.ResultString("result"));
    writer->Write("\n]", true);
    return false;
  };
  new HTTPChunkedWriter(response, "application/json", WriteFunc);
}
void AnalyzerSoftRates::AddRate(const json &params, Response &response) const {
  if((params.contains("Group") == false) || (params["Group"].is_number_integer() == false) || (params["Group"] < 0) || (params["Group"] > 10)) return HTTPServer::FillResponse(response, "'Group' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint8_t Group  = params["Group"];
  json    Config = Rate::CheckConfig(params, Group == 0);
  if(Config.is_string()) return HTTPServer::FillResponse(response, Config, "text/plain", HTTP_BAD_REQUEST);
  string Name      = Config["Name"];
  string StartDate = Config["StartDate"];
  string EndDate   = Config["EndDate"];
  string RateID    = Tools::GetUUID();
  Config.erase("Name");
  Config.erase("StartDate");
  Config.erase("EndDate");
  Database DB(pSerial);
  bool Result = DB.ExecSentenceNoResult("INSERT INTO rates (id, name, ordering, startdate, enddate, template) VALUES (" + DB.Bind(RateID) + ", " + DB.Bind(Name) + ", " + DB.Bind(Group) + ", to_date(" + DB.Bind(StartDate) + ", 'DD/MM/YYYY'), to_date(" + DB.Bind(EndDate) + ", 'DD/MM/YYYY'), " + DB.Bind(Config.dump()) + ")");
  HTTPServer::FillResponse(response, "", "application/json", Result ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoftRates::GetRate(const json &params, Response &response) const {
  if((params.contains("RateID") == false) || (params["RateID"].is_string() == false)) return HTTPServer::FillResponse(response, "'RateID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string ID = params["RateID"];
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT jsonb_build_object('Name', name, 'Group', ordering, 'StartDate', to_char(startdate, 'dd/mm/yyyy'), 'EndDate', to_char(enddate, 'dd/mm/yyyy')) || template::jsonb AS result FROM rates WHERE id = " + DB.Bind(ID));
  if(DB.FecthRow() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void AnalyzerSoftRates::SetRate(const json &params, Response &response) const {
  if((params.contains("RateID") == false) || (params["RateID"].is_string() == false)) return HTTPServer::FillResponse(response, "'RateID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Group") == false) || (params["Group"].is_number_integer() == false) || (params["Group"] < 0) || (params["Group"] > 10)) return HTTPServer::FillResponse(response, "'Group' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint8_t Group  = params["Group"];
  string  RateID = params["RateID"];
  json    Config = Rate::CheckConfig(params, Group == 0);
  if(Config.is_string()) return HTTPServer::FillResponse(response, Config, "text/plain", HTTP_BAD_REQUEST);
  string Name      = Config["Name"];
  string StartDate = Config["StartDate"];
  string EndDate   = Config["EndDate"];
  Config.erase("Name");
  Config.erase("StartDate");
  Config.erase("EndDate");
  Database DB(pSerial);
  bool Result = DB.ExecSentenceNoResult("UPDATE rates SET name = " + DB.Bind(Name) + ", ordering = " + DB.Bind(Group) + ", startdate = to_date(" + DB.Bind(StartDate) + ", 'DD/MM/YYYY'), enddate = to_date(" + DB.Bind(EndDate) + ", 'DD/MM/YYYY'), template = " + DB.Bind(Config.dump()) + " WHERE id = " + DB.Bind(RateID));
  HTTPServer::FillResponse(response, "", "application/json", Result ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoftRates::DelRate(const json &params, Response &response) const {
  if((params.contains("RateID") == false) || (params["RateID"].is_string() == false)) return HTTPServer::FillResponse(response, "'RateID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string ID = params["RateID"];
  Database DB(pSerial);
  bool Result = DB.ExecSentenceNoResult("DELETE FROM rates WHERE id = " + DB.Bind(ID));
  HTTPServer::FillResponse(response, "", "application/json", Result ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoftRates::GetCosts(const json &params, Response &response) const {
  AnalyzerSoftRatesEval Tmp(pSerial, pTimeZone);
  Tmp.GetCosts(params, response);
}
void AnalyzerSoftRates::GetBills(const json &params, Response &response) const {
  AnalyzerSoftRatesEval Tmp(pSerial, pTimeZone);
  Tmp.GetBills(params, response);
}
// ---------------------------------------------------------------------------------------------------------------------