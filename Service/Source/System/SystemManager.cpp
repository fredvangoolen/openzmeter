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
#ifdef OPENWRT // ======================================================================================================
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include "LogFile.h"
#include "MessageLoop.h"
#include "Status.h"
// ---------------------------------------------------------------------------------------------------------------------
SystemManager::SystemManager() {
  MessageLoop::RegisterListener("SystemManager_PeriodicTasks", PeriodicTasks, 5000);
  MessageLoop::RegisterListener("PowerOff", PowerOff);
}
SystemManager::~SystemManager() {
  MessageLoop::UnregisterListener("SystemManager_PeriodicTasks");
  MessageLoop::UnregisterListener("PowerOff");
}
uint64_t SystemManager::ParseProcMem(const string &name, const string &buff) {
  auto Pos = buff.find(name + ":");
  if(Pos == buff.npos) return 0;
  long Val = stoi(buff.substr(Pos + name.size() + 1));
  return Val * 1024;
}
void SystemManager::PeriodicTasks() {
  struct sysinfo Info;
  int Error1 = sysinfo(&Info);
  if(Error1 != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  struct statvfs Disk;
  int Error2 = statvfs("/", &Disk);
  if(Error2 != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  string MemResult = Exec("cat /proc/meminfo");
  if(MemResult.empty() == true) LogFile::Error("Can not read memory usage stats");
  string TempResult = Exec("cat /sys/class/thermal/thermal_zone0/temp");
  if(TempResult.empty() == true) LogFile::Error("Can not read temperature");
  Status::SetValue("/Uptime", (Error1 != 0) ? 0 : Info.uptime);
  Status::SetValue("/Swap_Total", (Error1 != 0) ? 0 : (uint64_t)Info.totalswap * Info.mem_unit);
  Status::SetValue("/Swap_Usage", (Error1 != 0) ? NAN : ((Info.totalswap - Info.freeswap) * 100.0) / Info.totalswap);
  Status::SetValue("/CPU_Usage", (Error1 != 0) ? NAN : ((Info.loads[0] / (double)(1 << SI_LOAD_SHIFT)) / get_nprocs()) * 100.0);
  Status::SetValue("/Disk_Total", (Error2 != 0) ? 0 : (uint64_t)Disk.f_blocks * Disk.f_frsize);
  Status::SetValue("/Disk_Usage", (Error2 != 0) ? NAN : ((Disk.f_blocks - Disk.f_bfree) * 100.0) / Disk.f_blocks);
  Status::SetValue("/RAM_Total", ParseProcMem("MemTotal", MemResult));
  Status::SetValue("/RAM_Usage", ((ParseProcMem("MemTotal", MemResult) -  ParseProcMem("MemFree", MemResult) - ParseProcMem("Cached", MemResult) - ParseProcMem("Buffers", MemResult)) * 100.0) / ParseProcMem("MemTotal", MemResult));
  Status::SetValue("/Temperature", (TempResult.empty()) ? json(nullptr) : json(stoi(TempResult) / 1000.0));
  Status::SetValue("/TimeSet", Tools::GetTime64() > 946684800000);
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
#else // ===============================================================================================================
SystemManager::SystemManager() {
}
SystemManager::~SystemManager() {
}
#endif // ==============================================================================================================