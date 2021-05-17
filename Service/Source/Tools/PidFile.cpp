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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "PidFile.h"
#include "nlohmann/json.hpp"
#include "ConfigFile.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
PidFile::PidFile() {
  LogFile::Info("Starting PidFile...");
  pFile = "/var/run/openZmeter.pid";
  json Config = ConfigFile::ReadConfig("/PidFile");
  if(Config.is_string()) pFile = Config;
  char PID_Number[16];
  int Len = sprintf(PID_Number, "%d", getpid());
  if(!pFile.empty()) remove(pFile.c_str());
  int File = open(pFile.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  if(File < 0) LogFile::Warning("Can not create PID file (%s)", strerror(errno));
  if(write(File, PID_Number, Len) <= 0) LogFile::Warning("Can not create PID file (%s)", strerror(errno));
  if(File > 0) close(File);
  LogFile::Info("PidFile started");
}
PidFile::~PidFile() {
  LogFile::Info("Stoping PidFile...");
  if(!pFile.empty()) remove(pFile.c_str());
  LogFile::Info("PidFile stoped");
}