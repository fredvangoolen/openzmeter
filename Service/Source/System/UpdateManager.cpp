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
#ifdef OPENWRT // ======================================================================================================
#include "SystemManager.h"
UpdateManager::UpdateManager() {
  HTTPServer::RegisterCallback("getUpdates",   GetUpdatesCallback);
  HTTPServer::RegisterCallback("setUpgrade",   SetUpgradeCallback);
}
UpdateManager::~UpdateManager() {
  HTTPServer::UnregisterCallback("getUpdates");
  HTTPServer::UnregisterCallback("setUpgrade");
}
void UpdateManager::GetUpdatesCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  int ResultCode = 0;
  if(params.contains("Force") && params["Force"].is_boolean() && (params["Force"] == true)) {
    SystemManager::Exec("opkg update", &ResultCode);
    if(ResultCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  string Result = SystemManager::Exec("opkg list-upgradable", &ResultCode);
  if(ResultCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  json Response = json::array();
  size_t Pos = 0;
  while(Pos < Result.size()) {
    json Update = json::object();
    size_t Pos2 = Result.find(" - ", Pos);
    Update["Package"] = Result.substr(Pos, Pos2 - Pos);
    Pos = Pos2 + 3;
    Pos2 = Result.find(" - ", Pos);
    Update["From"] = Result.substr(Pos, Pos2 - Pos);
    Pos = Pos2 + 3;
    Pos2 = Result.find("\n", Pos);
    if(Pos2 == string::npos) Pos2 = Result.size();
    Update["To"] = Result.substr(Pos, Pos2 - Pos);
    Pos = Pos2 + 1;
    Response.push_back(Update);
  }
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void UpdateManager::SetUpgradeCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Package") == false) || (params["Package"].is_string() == false)) return HTTPServer::FillResponse(response, "'Package' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Package = params["Package"];
  int ResultCode = 0;
  string Result = SystemManager::Exec("opkg upgrade --force-depends " + Package, &ResultCode);
  HTTPServer::FillResponse(response, "", "text/plain", (ResultCode == 0) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
#else // ===============================================================================================================
UpdateManager::UpdateManager() {
}
UpdateManager::~UpdateManager() {
}
#endif // ==============================================================================================================