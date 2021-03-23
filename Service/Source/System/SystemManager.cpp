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
#include "SystemManager.h"
#include "LogFile.h"
#include <sys/param.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include "MessageLoop.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
json            SystemManager::pStatus = json::object();
pthread_mutex_t SystemManager::pMutex  = PTHREAD_MUTEX_INITIALIZER;
// ---------------------------------------------------------------------------------------------------------------------
SystemManager::SystemManager() {
  MessageLoop::RegisterListener("SystemManager_PeriodicTasks", PeriodicTasks, 5000);
  MessageLoop::RegisterListener("PowerOff", PowerOff);
  HTTPServer::RegisterCallback("getSystem", GetSystemCallback, "API/GetSystem.json");
}
SystemManager::~SystemManager() {
  MessageLoop::UnregisterListener("SystemManager_PeriodicTasks");
  MessageLoop::UnregisterListener("PowerOff");
  HTTPServer::UnregisterCallback("getSystem");
}
uint64_t SystemManager::ParseProcMem(const string &name, const string &buff) {
  auto Pos = buff.find(name + ":");
  if(Pos == buff.npos) return 0;
  long Val = stoi(buff.substr(Pos + name.size() + 1));
  return Val * 1024;
}
bool SystemManager::ReadFile(const string filename, string &res) {
  res.clear();
  int File = open(filename.c_str(), O_RDONLY);
  if(File == -1) return false;
  char Tmp[128];
  while(true) {
    ssize_t Bytes = read(File, Tmp, sizeof(Tmp));
    if(Bytes <= 0) {
      close(File);
      return (Bytes == 0);
    }
    res.append(Tmp, Bytes);
  }
}
void SystemManager::PeriodicTasks() {
  json   Status = json::object();
  string File;
  if(ReadFile("/proc/stat", File) == true) {
    istringstream Stream(File);
    string  Text;
    uint8_t Field = 0;
    float   Usage = 0.0;
    while(getline(Stream, Text, ' ')) {
      if(Text.empty()) continue;
      if((Field > 0) && (Field < 4)) Usage += atoi(Text.c_str());
      if(Field == 4) {
        Status["CPU"]["Usage"] = (Usage * 100.0) / atoi(Text.c_str());
        break;
      }
      Field++;
    }
    Status["CPU"]["Count"] = get_nprocs();
    if(ReadFile("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq", File) == true) {
      Status["CPU"]["Frequency"] = atoi(File.c_str()) / 1000.0;
    }
  }
  if(ReadFile("/proc/uptime", File) == true) {
    istringstream Stream(File);
    string  Text;
    getline(Stream, Text, ' ');
    Status["Uptime"] = (uint64_t)(atof(Text.c_str()) * 1000);
  }
  if(ReadFile("/proc/meminfo", File) == true) {
    Status["RAM"] = json::object();
    Status["RAM"]["Total"] = ParseProcMem("MemTotal", File);
    Status["RAM"]["Free"] = ParseProcMem("MemFree", File);
    Status["RAM"]["Cached"] = ParseProcMem("Cached", File);
    Status["RAM"]["Buffers"] = ParseProcMem("Buffers", File);
    Status["Swap"] = json::object();
    Status["Swap"]["Total"] = ParseProcMem("SwapTotal", File);
    Status["Swap"]["Free"] = ParseProcMem("SwapFree", File);
  }
  struct statvfs Disk;
  if(statvfs("/", &Disk) == 0) {
    Status["Storage"] = json::object();
    Status["Storage"]["Total"] = (uint64_t)Disk.f_blocks * Disk.f_frsize;
    Status["Storage"]["Free"] = (uint64_t)Disk.f_bfree * Disk.f_frsize;
  }
  if(ReadFile("/sys/class/thermal/thermal_zone0/temp", File) == true) {
    Status["Temperature"] = atoi(File.c_str()) / 1000.0;
  }
  pthread_mutex_lock(&pMutex);
  pStatus = Status;
  pthread_mutex_unlock(&pMutex);
}
void SystemManager::PowerOff() {
  Exec("poweroff");
}
string SystemManager::Exec(const string &command, int *resultCode) {
  char Buffer[128];
  string Result = "";
  FILE *Pipe = popen(command.c_str(), "r");
  if(!Pipe) {
    LogFile::Error("Can not exec '%s' (%s)", command.c_str(), strerror(errno));
    if(resultCode != NULL) *resultCode = -1;
    return "";
  }
  while(fgets(Buffer, sizeof(Buffer), Pipe) != NULL) Result += Buffer;
  int Code = WEXITSTATUS(pclose(Pipe));
  if(resultCode != NULL) *resultCode = Code;
  return Result;
}
#ifdef OPENWRT
json SystemManager::uBusCall(const string &path, const string &method, const json &params) {
  string Res = Exec("ubus call " + path + " " + method + " " + ((params.is_null() == true) ? "" : "'" + params.dump() + "'"));
  if(Res.empty() == true) return nullptr;
  json Tmp = json::parse(Res, NULL, false);
  if(Tmp.is_discarded() == true) return nullptr;
  return Tmp;
}
#endif
json SystemManager::Status() {
  pthread_mutex_lock(&pMutex);
  json Status = pStatus;
  pthread_mutex_unlock(&pMutex);
  return Status;
}
string SystemManager::SystemLog() {
#ifdef OPENWRT
  return Exec("logread");
#else
  return "";
#endif
}
void SystemManager::GetSystemCallback(HTTPRequest &req) {
/*  string UserName = req.UserName();
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is required", "text/plain", HTTP_BAD_REQUEST);
  vector<string> Series;
  for(auto &Serie : params["Series"]) {
    if(Serie.is_string() == false) return HTTPServer::FillResponse(response, "'Series' param is required", "text/plain", HTTP_BAD_REQUEST);
    Series.push_back(Serie);
  }
  json Result = json::object();
  for(auto &Serie : Series) {
    if(Serie == "Log") {
      Result["Log"] = LogFile::ToJSON();
    } else if(Serie == "Status") {
      Result["Status"] = SystemManager::Status();
    } else if(Serie == "SystemLog") {
      Result["SystemLog"] = SystemManager::SystemLog();
    } else {
      return HTTPServer::FillResponse(response, "'Series' param is required", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);*/
}
// ---------------------------------------------------------------------------------------------------------------------