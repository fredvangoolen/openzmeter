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
  pSerial      = id;
  pName        = "Unnamed analyzer";
  pLastSave    = 0;
}
string Analyzer::Serial() const {
  return pSerial;
}
Analyzer::~Analyzer() {
//  pInstances.WaitInstances();
//  StopSync();
}
json Analyzer::GetJSONShortDescriptor() {
//  InstanceLocks::Allocator Reference(&pInstances);
  json Response;
  pConfigLocks.ReadLock();
  Response["Analyzer"] = pSerial;
  Response["Name"] = pName;
  Response["Online"] = ((Tools::GetTime() - pLastSave) < 20000);
  pConfigLocks.ReadUnlock();
  return Response;
}
bool Analyzer::CheckRights(HTTPRequest &req, const uint neededAccess) const {
//  InstanceLocks::Allocator Reference(&pInstances);
// req.FromString("", MHD_HTTP_UNAUTHORIZED);
 return true;
}
//json Analyzer::GetJSONSyncConfig() {
//  InstanceLocks::Allocator Reference(&pInstances);
//  json Response = json::object();
//  pConfigLocks.ReadLock();
//  Response["RemoteEnabled"] = (pSyncThread != 0);
//  Response["RemoteServer"] = pSyncServer;
//  Response["RemoteUser"] = pSyncUser;
//  Response["RemotePass"] = pSyncPass;
//  pConfigLocks.ReadUnlock();
//  return Response; 
//}
//void Analyzer::StopSync() {
//  pConfigLocks.WriteLock();
//  pTerminateSync = true;
//  if(pSyncThread != 0) pthread_join(pSyncThread, NULL);
//  pSyncThread = 0;
//  pConfigLocks.WriteUnlock();
//}