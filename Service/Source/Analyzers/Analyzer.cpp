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
#include <math.h>
#include <sys/types.h>
#include <string.h>
#include "Analyzer.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
Analyzer::Analyzer(const string &id) : Serial(id) {
  pName     = "Unnamed analyzer";
  pLastSave = 0;
}
Analyzer::~Analyzer() {
}
json Analyzer::GetJSONDescriptor() {
  json Response = json::object();
  Response["Analyzer"] = Serial;
  pMembersLocks.ReadLock();
  Response["Name"] = pName;
  Response["Online"] = ((Tools::GetTime64() - pLastSave) < 20000);
  pMembersLocks.Unlock();
  return Response;
}
string Analyzer::GetName() {
  pMembersLocks.ReadLock();
  string Name = pName;
  pMembersLocks.Unlock();
  return Name;
}
void Analyzer::SetName(const string &name) {
  pMembersLocks.WriteLock();
  pName = name;
  pMembersLocks.Unlock();
}
void Analyzer::SetOnline() {
  pMembersLocks.WriteLock();
  pLastSave = Tools::GetTime64();
  pMembersLocks.Unlock();
}
bool Analyzer::IsOnline() {
  pMembersLocks.ReadLock();
  bool Online = ((Tools::GetTime64() - pLastSave) < 20000);
  pMembersLocks.Unlock();
  return Online;
}
bool Analyzer::CheckRights(const string &user, const AnalyzerRights neededAccess) {
  if(user == "admin") return true;
  Database DB;
  string Permision = (neededAccess == READ) ? "read" : (neededAccess == WRITE) ? "write" : "config";
  DB.ExecSentenceAsyncResult("SELECT " + Permision + " AS result FROM permissions WHERE username = " + DB.Bind(user) + " AND serial = " + DB.Bind(Serial));
  bool Result = ((DB.FecthRow() == true) && (DB.ResultString("result") == "t"));
  return Result;
}
void Analyzer::GetPermissions(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('User', username, 'Read', read, 'Write', write, 'Configure', config))), '[]') AS result FROM permissions WHERE serial = " + DB.Bind(Serial));
  if(DB.FecthRow() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void Analyzer::AddPermission(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  string User   = params["User"];
  bool   Read   = params["Read"];
  bool   Write  = params["Write"];
  bool   Config = params["Configure"];
  bool   Result = DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config) VALUES(" + DB.Bind(User) + ", " + DB.Bind(Serial) + ", " + DB.Bind(Read) + ", " + DB.Bind(Write) + ", " + DB.Bind(Config) + ")");
  HTTPServer::FillResponse(response, "", "application/json", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void Analyzer::SetPermission(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  string User   = params["User"];
  bool   Read   = params["Read"];
  bool   Write  = params["Write"];
  bool   Config = params["Configure"];
  bool   Result = DB.ExecSentenceNoResult("UPDATE permissions SET read = " + DB.Bind(Read) + ", write = " + DB.Bind(Write) + ", config = " + DB.Bind(Config) + " WHERE username = " + DB.Bind(User) + " AND serial = " + DB.Bind(Serial));
  HTTPServer::FillResponse(response, "", "application/json", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void Analyzer::DelPermission(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  string User   = params["User"];
  bool   Result = DB.ExecSentenceNoResult("DELETE FROM permissions WHERE userename = " + DB.Bind(User) + " AND serial = " + DB.Bind(Serial));
  HTTPServer::FillResponse(response, "", "application/json", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}