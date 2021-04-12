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
#include "UpdateManager.h"
#include "LogFile.h"
#include "SystemManager.h"
//----------------------------------------------------------------------------------------------------------------------
#ifdef OPENWRT
string UpdateManager::pUpdateURL = "https://www.openzmeter.com/update/";
#endif
// ---------------------------------------------------------------------------------------------------------------------
UpdateManager::UpdateManager() {
#ifdef OPENWRT
  LogFile::Info("Starting UpdateManager...");
  HTTPServer::RegisterCallback("getUpdates",   GetUpdatesCallback);
  HTTPServer::RegisterCallback("setUpgrade",   SetUpgradeCallback);
  LogFile::Info("UpdateManager started");
#endif
}
UpdateManager::~UpdateManager() {
#ifdef OPENWRT
  LogFile::Info("UpdateManager", "Stopping Update manager...");
  HTTPServer::UnregisterCallback("getUpdates");
  HTTPServer::UnregisterCallback("setUpgrade");
  LogFile::Info("UpdateManager", "Update manager stoped");
#endif
}
#ifdef OPENWRT
void UpdateManager::GetUpdatesCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  // EL JSON TIENE ESTA FORMA
  // {
  //  "devel": {
//      "1231231.3423": {
//        "ChangeLog":"dsdfsfdsf"
//        "arm_....": "HTTPS://......"
//      }
//    }
  //}
//me descargo el JSON de versiones
//busco mi arquitectura
//si esta devuelvo el changelog

//  int ResultCode = 0;
//  if(params.contains("Force") && params["Force"].is_boolean() && (params["Force"] == true)) {
//    SystemManager::Exec("opkg update", &ResultCode);
//    if(ResultCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
//  }
//  string Result = SystemManager::Exec("opkg list-upgradable", &ResultCode);
//  if(ResultCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
//  json Response = json::array();
//  size_t Pos = 0;
//  while(Pos < Result.size()) {
//    json Update = json::object();
//    size_t Pos2 = Result.find(" - ", Pos);
//    Update["Package"] = Result.substr(Pos, Pos2 - Pos);
//    Pos = Pos2 + 3;
//    Pos2 = Result.find(" - ", Pos);
//    Update["From"] = Result.substr(Pos, Pos2 - Pos);
//    Pos = Pos2 + 3;
//    Pos2 = Result.find("\n", Pos);
//    if(Pos2 == string::npos) Pos2 = Result.size();
//    Update["To"] = Result.substr(Pos, Pos2 - Pos);
//    Pos = Pos2 + 1;
//    Response.push_back(Update);
//  }
//  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void UpdateManager::SetUpgradeCallback(HTTPRequest &req) {
//  if(req.UserName() != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
//  if((params.contains("Packages") == false) || (params["Packages"].is_array() == false) || (params["Packages"].size() == 0)) return HTTPServer::FillResponse(response, "'Packages' param is required", "text/plain", HTTP_BAD_REQUEST);
//  string Command = "opkg upgrade --force-depends";
//  for(auto &Package : params["Packages"]) {
//    if(Package.is_string() == false) return HTTPServer::FillResponse(response, "'Packages' param is required", "text/plain", HTTP_BAD_REQUEST);
//    Command.append(" " + Package.get<string>());
//  }
//  auto Func = [Command](uint64_t offset, DataSink &sink) {
//    char   Buffer[128];
//    string Result = "";
//    FILE  *Pipe   = popen(Command.c_str(), "r");
//    if(!Pipe) {
//      LogFile::Error("Can not exec 'opkg upgrade' (%s)", strerror(errno));
//      sink.os << "event: status\ndata: ERROR (Can not exec 'opkg upgrade')\n\n";
//      return false;
//    }
//    while((fgets(Buffer, sizeof(Buffer), Pipe) != NULL) && sink.is_writable()) sink.os << "event: text\ndata: " + string(Buffer) + "\n\n";
//    int Code = WEXITSTATUS(pclose(Pipe));
//    sink.os << ((Code == 0) ? "event: status\ndata: OK (Upgrade finished)\n\n" : "event: status\ndata: ERROR (Upgrade failed)\n\n");
//    sink.done();
//    return true;
//  };
//  response.set_chunked_content_provider("text/event-stream", Func);
}
#endif
//----------------------------------------------------------------------------------------------------------------------