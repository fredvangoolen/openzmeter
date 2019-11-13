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
  pMembersLocks.ReadLock();
  //req.ExecSentenceAsyncResult("SELECT");  ...
  //  InstanceLocks::Allocator Reference(&pInstances);
// req.FromString("", MHD_HTTP_UNAUTHORIZED);
  pMembersLocks.ReadUnlock();
 return true;
}