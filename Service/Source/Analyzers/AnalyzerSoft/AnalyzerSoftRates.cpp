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
#include "AnalyzerSoftRatesEval.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftRates::AnalyzerSoftRates(const string &serial) {
  pSerial = serial;
}
AnalyzerSoftRates::~AnalyzerSoftRates() {
}
void AnalyzerSoftRates::Config(const string &timezone) {
  load_time_zone(timezone, &pTimeZone);
}
void AnalyzerSoftRates::GetRates(const json &params, Response &response) const {
  auto Func = [&](uint64_t offset, DataSink &sink) -> bool {
    uint32_t Items = 0;
    sink.os << "[\n  ";
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_object('RateID', id, 'Name', name, 'Group', ordering, 'StartDate', to_char(startdate, 'dd/mm/yyyy'), 'EndDate', to_char(enddate, 'dd/mm/yyyy')) AS result FROM rates");
    while(DB.FecthRow() && sink.is_writable()) sink.os << ((Items++ != 0) ? ",\n  " + DB.ResultString("result") : DB.ResultString("result"));
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerSoftRates::AddRate(const json &params, Response &response) const {
  json    Config = Rate::CheckConfig(params);
  if(Config.is_string()) return HTTPServer::FillResponse(response, Config, "text/plain", HTTP_BAD_REQUEST);
  string  Name      = Config["Name"];
  uint8_t Group     = Config["Group"];
  string  StartDate = Config["StartDate"];
  string  EndDate   = Config["EndDate"];
  string  RateID    = Tools::GetUUID();
  Config.erase("Name");
  Config.erase("Group");
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
  string  RateID = params["RateID"];
  json    Config = Rate::CheckConfig(params);
  if(Config.is_string()) return HTTPServer::FillResponse(response, Config, "text/plain", HTTP_BAD_REQUEST);
  string  Name      = Config["Name"];
  uint8_t Group     = Config["Group"];
  string  StartDate = Config["StartDate"];
  string  EndDate   = Config["EndDate"];
  Config.erase("Name");
  Config.erase("Group");
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
  if((params.contains("To") == false) || (params["To"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(params["From"] > params["To"]) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Group") == false) || (params["Group"].is_number_integer() == false) || (params["Group"] < 0) || (params["Group"] > 10)) return HTTPServer::FillResponse(response, "'Group' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  uint8_t  Group  = params["Group"];
  uint16_t Series = AnalyzerSoftRatesEval::SERIE_TIMESTAMP;
  for(auto &Serie : params["Series"]) {
    if(Serie.is_string() == false) {
      return HTTPServer::FillResponse(response, "'Series' must be array of strings", "text/plain", HTTP_BAD_REQUEST);
    } else if(Serie == "ACTIVE_POWER") {
      Series |= AnalyzerSoftRatesEval::SERIE_ACTIVE_POWER;
    } else if(Serie == "ACTIVE_ENERGY") {
      Series |= AnalyzerSoftRatesEval::SERIE_ACTIVE_ENERGY;
    } else if(Serie == "REACTIVE_POWER") {
      Series |= AnalyzerSoftRatesEval::SERIE_REACTIVE_POWER;
    } else if(Serie == "REACTIVE_ENERGY") {
      Series |= AnalyzerSoftRatesEval::SERIE_REACTIVE_ENERGY;
    } else if(Serie == "COST") {
      Series |= AnalyzerSoftRatesEval::SERIE_COST;
    } else if(Serie == "POWER_COST") {
      Series |= AnalyzerSoftRatesEval::SERIE_POWER_COST;
    } else if(Serie == "ENERGY_COST") {
      Series |= AnalyzerSoftRatesEval::SERIE_ENERGY_COST;
    } else if(Serie == "PERIOD") {
      Series |= AnalyzerSoftRatesEval::SERIE_PERIOD;
    } else if(Serie == "COLOR") {
      Series |= AnalyzerSoftRatesEval::SERIE_COLOR;
    } else if(Serie == "CONTRACTED") {
      Series |= AnalyzerSoftRatesEval::SERIE_CONTRACTED;
    } else if(Serie == "BILL") {
      Series |= AnalyzerSoftRatesEval::SERIE_BILL;
    } else {
      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  auto Func = [&, From, To, Group, Series](uint64_t offset, DataSink &sink) -> bool {
    AnalyzerSoftRatesEval Eval(pSerial, pTimeZone, From, To, Group, Series);
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
void AnalyzerSoftRates::GetBills(const json &params, Response &response) const {
  if((params.contains("To") == false) || (params["To"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(params["From"] > params["To"]) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Group") == false) || (params["Group"].is_number_integer() == false) || (params["Group"] < 0) || (params["Group"] > 10)) return HTTPServer::FillResponse(response, "'Group' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t From  = params["From"];
  uint64_t To    = params["To"];
  uint8_t  Group = params["Group"];
  auto Func = [&, From, To, Group](uint64_t offset, DataSink &sink) -> bool {
    uint32_t Items = 0;
    AnalyzerSoftRatesEval Eval(pSerial, pTimeZone, From, To, Group);
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