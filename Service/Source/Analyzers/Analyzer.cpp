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
bool Analyzer::CheckRights(HTTPRequest &req, const uint neededAccess) {
  if(req.UserName() == "admin") return true;
  string Permision = (neededAccess == READ) ? "Read" : (neededAccess == WRITE) ? "Write" : "Config";
  req.ExecSentenceAsyncResult("SELECT " + req.Bind(Permision) + " AS result FROM permissions WHERE username = " + req.Bind(req.UserName()) + " AND serial = " + req.Bind(pSerial));
  if((req.FecthRow() == true) && (req.ResultString("result") == "t")) return true;
  req.FromString("", MHD_HTTP_FORBIDDEN);
  return false;
}
void Analyzer::GetPermissions(HTTPRequest &req) {
  if(CheckRights(req, CONFIG) == false) return;
  req.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('User', username, 'Read', read, 'Write', write, 'Configure', config))), '[]') AS result FROM permisions WHERE serial = " + req.Bind(pSerial));
  if(req.FecthRow() == false) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("result"), MHD_HTTP_OK);
}
void Analyzer::AddPermission(HTTPRequest &req) {
  if(CheckRights(req, CONFIG) == false) return;
  json Params = req.GetParams();
  string User = Params["User"];
  bool Read = Params["Read"];
  bool Write = Params["Write"];
  bool Config = Params["Configure"];
  bool Result = req.ExecSentenceNoResult("INSERT INTO permisions(username, serial, read, write, config) VALUES(" + req.Bind(User) + ", " + req.Bind(pSerial) + ", " + req.Bind(Read) + ", " + req.Bind(Write) + ", " + req.Bind(Config) + ")");
  req.FromString((Result == true) ? "{}" : "", (Result == true) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
}
void Analyzer::SetPermission(HTTPRequest &req) {

}
void Analyzer::DelPermission(HTTPRequest &req) {

}