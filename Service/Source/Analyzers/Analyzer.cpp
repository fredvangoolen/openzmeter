/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.cpp
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/
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
  pMembersLocks.ReadLock();
  Response["Analyzer"] = Serial;
  Response["Name"] = pName;
  Response["Online"] = ((Tools::GetTime() - pLastSave) < 20000);
  pMembersLocks.ReadUnlock();
  return Response;
}
bool Analyzer::CheckRights(const string &user, const AnalyzerRights neededAccess) {
  if(user == "admin") return true;
  Database DB;
  string Permision = (neededAccess == READ) ? "read" : (neededAccess == WRITE) ? "write" : "config";
  DB.ExecSentenceAsyncResult("SELECT " + DB.Bind(Permision) + " AS result FROM permissions WHERE username = " + DB.Bind(user) + " AND serial = " + DB.Bind(Serial));
  return ((DB.FecthRow() == true) && (DB.ResultString("result") == "t"));
}
void Analyzer::GetPermissions(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('User', username, 'Read', read, 'Write', write, 'Configure', config))), '[]') AS result FROM permisions WHERE serial = " + DB.Bind(Serial));
  if(DB.FecthRow() == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPCallback::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void Analyzer::AddPermission(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  string User   = params["User"];
  bool   Read   = params["Read"];
  bool   Write  = params["Write"];
  bool   Config = params["Configure"];
  bool   Result = DB.ExecSentenceNoResult("INSERT INTO permisions(username, serial, read, write, config) VALUES(" + DB.Bind(User) + ", " + DB.Bind(Serial) + ", " + DB.Bind(Read) + ", " + DB.Bind(Write) + ", " + DB.Bind(Config) + ")");
  HTTPCallback::FillResponse(response, "", "application/json", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void Analyzer::SetPermission(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  string User   = params["User"];
  bool   Read   = params["Read"];
  bool   Write  = params["Write"];
  bool   Config = params["Configure"];
  bool   Result = DB.ExecSentenceNoResult("UPDATE permisions SET read = " + DB.Bind(Read) + ", write = " + DB.Bind(Write) + ", config = " + DB.Bind(Config) + " WHERE username = " + DB.Bind(User) + " AND serial = " + DB.Bind(Serial));
  HTTPCallback::FillResponse(response, "", "application/json", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void Analyzer::DelPermission(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  string User   = params["User"];
  bool   Result = DB.ExecSentenceNoResult("DELETE FROM permisions WHERE userename = " + DB.Bind(User) + " AND serial = " + DB.Bind(Serial));
  HTTPCallback::FillResponse(response, "", "application/json", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}