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

#ifdef OPENWRT // ======================================================================================================
#include <sys/param.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include "MessageLoop.h"
// ---------------------------------------------------------------------------------------------------------------------
json            SystemManager::pStatus = json::object();
pthread_mutex_t SystemManager::pMutex  = PTHREAD_MUTEX_INITIALIZER;
// ---------------------------------------------------------------------------------------------------------------------
SystemManager::SystemManager() {
  MessageLoop::RegisterListener("SystemManager_PeriodicTasks", PeriodicTasks, 5000);
  MessageLoop::RegisterListener("PowerOff", PowerOff);
  HTTPServer::RegisterCallback("getSystem", GetSystemCallback);
  HTTPServer::RegisterCallback("getLog", GetLogCallback);
}
SystemManager::~SystemManager() {
  MessageLoop::UnregisterListener("SystemManager_PeriodicTasks");
  MessageLoop::UnregisterListener("PowerOff");
  HTTPServer::UnregisterCallback("getSystem");
  HTTPServer::UnregisterCallback("getLog");
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
  json           Status = json::object();
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
    Status["Uptime"] = atof(Text.c_str());
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
  Status["Time"] = Tools::GetTime64();
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
json SystemManager::uBusCall(const string &path, const string &method, const json &params) {
  string Res = Exec("ubus call " + path + " " + method + " " + ((params.is_null() == true) ? "" : "'" + params.dump() + "'"));
  if(Res.empty() == true) return nullptr;
  json Tmp = json::parse(Res, NULL, false);
  if(Tmp.is_discarded() == true) return nullptr;
  return Tmp;
}
void SystemManager::GetSystemCallback(const json &params, Response &response) {
  pthread_mutex_lock(&pMutex);
  HTTPServer::FillResponse(response, pStatus.dump(), "application/json", HTTP_OK);
  pthread_mutex_unlock(&pMutex);
}
json SystemManager::Status() {
  pthread_mutex_lock(&pMutex);
  json Status = pStatus;
  pthread_mutex_unlock(&pMutex);
  return Status;
}
void SystemManager::GetLogCallback(const json &params, Response &response) {
  HTTPServer::FillResponse(response, LogFile::ToJSON().dump(), "application/json", HTTP_OK);
}
#else // ===============================================================================================================
SystemManager::SystemManager() {
}
SystemManager::~SystemManager() {
}
json SystemManager::Status() {
  return json::object();
}
#endif // ==============================================================================================================