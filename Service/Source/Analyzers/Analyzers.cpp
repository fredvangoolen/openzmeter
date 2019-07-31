/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.cpp
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/ 
#include <math.h>
#include <sys/types.h>
#include <string.h>
#include "Analyzers.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
Analyzer::Analyzer(const string &id) {
  pSerial = id;
  pName = "Unnamed analyzer";
  pLastSave = 0;
}
Analyzer::~Analyzer() {
}
string Analyzer::Serial() const {
  return pSerial;
}
json Analyzer::GetJSONShortDescriptor() const {
  json Response;
  Response["ID"] = pSerial;
  Response["Name"] = pName;
  Response["Online"] = ((Tools::GetTime() - pLastSave) < 20000);
  return Response;
}
bool Analyzer::CheckRights(HTTPRequest &req, const uint neededAccess) const {
// req.FromString("", MHD_HTTP_UNAUTHORIZED);
 return true;
}
