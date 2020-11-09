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
#include "System.h"
#ifdef OPENWRT // ======================================================================================================
#include <sys/param.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include "LogFile.h"
#include "MessageLoop.h"
#include "Status.h"
// ---------------------------------------------------------------------------------------------------------------------
System::System() {
  MessageLoop::RegisterListener("SystemManager_PeriodicTasks", PeriodicTasks, 5000);
  MessageLoop::RegisterListener("PowerOff", PowerOff);
}
System::~System() {
  MessageLoop::UnregisterListener("SystemManager_PeriodicTasks");
  MessageLoop::UnregisterListener("PowerOff");
}
uint64_t System::ParseProcMem(const string &name, const string &buff) {
  auto Pos = buff.find(name + ":");
  if(Pos == buff.npos) return 0;
  long Val = stoi(buff.substr(Pos + name.size() + 1));
  return Val * 1024;
}
bool System::ReadFile(const string filename, string &res) {
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
void System::PeriodicTasks() {
  struct sysinfo Info;
  if(sysinfo(&Info) == 0) {
    Status::SetValue("/Uptime", Info.uptime);
    Status::SetValue("/Swap_Total", (uint64_t)Info.totalswap * Info.mem_unit);
    Status::SetValue("/Swap_Usage", ((Info.totalswap - Info.freeswap) * 100.0) / Info.totalswap);
    Status::SetValue("/CPU_Usage", MIN(100.0, ((Info.loads[0] / (double)(1 << SI_LOAD_SHIFT)) / get_nprocs()) * 100.0));
  } else {
    Status::SetValue("/Uptime", nullptr);
    Status::SetValue("/Swap_Total", nullptr);
    Status::SetValue("/Swap_Usage", nullptr);
    Status::SetValue("/CPU_Usage", nullptr);
  }
  struct statvfs Disk;
  if(statvfs("/", &Disk) == 0) {
    Status::SetValue("/Disk_Total", (uint64_t)Disk.f_blocks * Disk.f_frsize);
    Status::SetValue("/Disk_Usage", ((Disk.f_blocks - Disk.f_bfree) * 100.0) / Disk.f_blocks);
  } else {
    Status::SetValue("/Disk_Total", nullptr);
    Status::SetValue("/Disk_Usage", nullptr);
  }
  string Memory;
  if(ReadFile("/proc/meminfo", Memory) == true) {
    uint64_t MemTotal = ParseProcMem("MemTotal", Memory);
    Status::SetValue("/RAM_Total", MemTotal);
    Status::SetValue("/RAM_Usage", ((MemTotal -  ParseProcMem("MemFree", Memory) - ParseProcMem("Cached", Memory) - ParseProcMem("Buffers", Memory)) * 100.0) / ParseProcMem("MemTotal", Memory));
  } else {
    Status::SetValue("/RAM_Total", nullptr);
    Status::SetValue("/RAM_Usage", nullptr);
  }
  string Temperature;
  if(ReadFile("/sys/class/thermal/thermal_zone0/temp", Temperature) == true) {
    Status::SetValue("/Temperature", (Temperature.empty()) ? json(nullptr) : json(stoi(Temperature) / 1000.0));
  } else {
    Status::SetValue("/Temperature", nullptr);
  }
  Status::SetValue("/TimeSet", Tools::GetTime64() > 946684800000);
}
void System::PowerOff() {
  Exec("poweroff");
}
string System::Exec(const string &command, int *resultCode) {
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
json System::uBusCall(const string &path, const string &method, const json &params) {
  string Res = Exec("ubus call " + path + " " + method + " " + ((params.is_null() == true) ? "" : "'" + params.dump() + "'"));
  if(Res.empty() == true) return nullptr;
  json Tmp = json::parse(Res, NULL, false);
  if(Tmp.is_discarded() == true) return nullptr;
  return Tmp;
}
#else // ===============================================================================================================
System::System() {
}
System::~System() {
}
#endif // ==============================================================================================================