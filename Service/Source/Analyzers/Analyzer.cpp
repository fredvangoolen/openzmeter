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
Analyzer::Analyzer(const string &id) {
  pSerial   = id;
  pName     = "Unnamed analyzer";
  pLastSave = 0;
}
string Analyzer::Serial() {
  pMembersLocks.ReadLock();
  string Serial = pSerial;
  pMembersLocks.ReadUnlock();
  return Serial;
}
Analyzer::~Analyzer() {
}
json Analyzer::GetJSONShortDescriptor() {
  json Response = json::object();
  pMembersLocks.ReadLock();
  Response["Analyzer"] = pSerial;
  Response["Name"] = pName;
  Response["Online"] = ((Tools::GetTime() - pLastSave) < 20000);
  pMembersLocks.ReadUnlock();
  return Response;
}
bool Analyzer::CheckRights(const json &params, Response &response, const uint neededAccess) {
  string UserName = params["UserName"];
  if(UserName == "admin") return true;
  string Permision = (neededAccess == READ) ? "Read" : (neededAccess == WRITE) ? "Write" : "Config";
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT " + DB.Bind(Permision) + " AS result FROM permissions WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
  if((DB.FecthRow() == true) && (DB.ResultString("result") == "t")) return true;
  response.set_content("", "text/plain");
  response.status = HTTP_FORBIDDEN;
  return false;
}
void Analyzer::GetPermissions(const json &params, Response &response) {
  if(CheckRights(params, response, CONFIG) == false) return;
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('User', username, 'Read', read, 'Write', write, 'Configure', config))), '[]') AS result FROM permisions WHERE serial = " + DB.Bind(pSerial));
  if(DB.FecthRow() == false) {
    response.set_content("", "text/plain");
    response.status = HTTP_INTERNAL_SERVER_ERROR;
  } else {
    response.set_content(DB.ResultString("result"), "application/json");
    response.status = HTTP_OK;
  }
}
void Analyzer::AddPermission(const json &params, Response &response) {
  if(CheckRights(params, response, CONFIG) == false) return;
  Database DB;
  string User   = params["User"];
  bool   Read   = params["Read"];
  bool   Write  = params["Write"];
  bool   Config = params["Configure"];
  bool   Result = DB.ExecSentenceNoResult("INSERT INTO permisions(username, serial, read, write, config) VALUES(" + DB.Bind(User) + ", " + DB.Bind(pSerial) + ", " + DB.Bind(Read) + ", " + DB.Bind(Write) + ", " + DB.Bind(Config) + ")");
  response.set_content("", "application/json");
  response.status = (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR;
}
void Analyzer::SetPermission(const json &params, Response &response) {

}
void Analyzer::DelPermission(const json &params, Response &response) {

}