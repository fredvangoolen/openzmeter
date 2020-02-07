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
  pMembersLocks = PTHREAD_RWLOCK_INITIALIZER;
}
Analyzer::~Analyzer() {
}
json Analyzer::GetJSONDescriptor() {
  json Response = json::object();
  Response["Analyzer"] = Serial;
  pthread_rwlock_rdlock(&pMembersLocks);
  Response["Name"] = pName;
  Response["Online"] = ((Tools::GetTime() - pLastSave) < 20000);
  pthread_rwlock_unlock(&pMembersLocks);
  return Response;
}
string Analyzer::GetName() {
  pthread_rwlock_rdlock(&pMembersLocks);
  string Name = pName;
  pthread_rwlock_unlock(&pMembersLocks);
  return Name;
}
void Analyzer::SetName(const string &name) {
  pthread_rwlock_wrlock(&pMembersLocks);
  pName = name;
  pthread_rwlock_unlock(&pMembersLocks);
}
void Analyzer::SetOnline() {
  pthread_rwlock_wrlock(&pMembersLocks);
  pLastSave = Tools::GetTime();
  pthread_rwlock_unlock(&pMembersLocks);
}
bool Analyzer::IsOnline() {
  pthread_rwlock_rdlock(&pMembersLocks);
  bool Online = ((Tools::GetTime() - pLastSave) < 20000);
  pthread_rwlock_unlock(&pMembersLocks);
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
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('User', username, 'Read', read, 'Write', write, 'Configure', config))), '[]') AS result FROM permissions WHERE serial = " + DB.Bind(Serial));
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
  bool   Result = DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config) VALUES(" + DB.Bind(User) + ", " + DB.Bind(Serial) + ", " + DB.Bind(Read) + ", " + DB.Bind(Write) + ", " + DB.Bind(Config) + ")");
  HTTPCallback::FillResponse(response, "", "application/json", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void Analyzer::SetPermission(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  string User   = params["User"];
  bool   Read   = params["Read"];
  bool   Write  = params["Write"];
  bool   Config = params["Configure"];
  bool   Result = DB.ExecSentenceNoResult("UPDATE permissions SET read = " + DB.Bind(Read) + ", write = " + DB.Bind(Write) + ", config = " + DB.Bind(Config) + " WHERE username = " + DB.Bind(User) + " AND serial = " + DB.Bind(Serial));
  HTTPCallback::FillResponse(response, "", "application/json", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void Analyzer::DelPermission(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  string User   = params["User"];
  bool   Result = DB.ExecSentenceNoResult("DELETE FROM permissions WHERE userename = " + DB.Bind(User) + " AND serial = " + DB.Bind(Serial));
  HTTPCallback::FillResponse(response, "", "application/json", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
