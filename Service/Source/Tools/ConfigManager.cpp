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
#include "ConfigManager.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
int             ConfigManager::pFile   = -1;
pthread_mutex_t ConfigManager::pMutex  = PTHREAD_MUTEX_INITIALIZER;
json            ConfigManager::pConfig = nullptr;
//----------------------------------------------------------------------------------------------------------------------
ConfigManager::ConfigManager(const string &name) {
  pthread_mutex_lock(&pMutex);
  pFile = open(name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if(pFile < 0) {
    LogFile::Error("Can not open config file (%s)", strerror(errno));
    pFile = -1;
    pthread_mutex_unlock(&pMutex);
    return;
  }
  string Read;
  char Buff[256];
  ssize_t Len;
  do {
    Len = read(pFile, Buff, sizeof(Buff));
    Read += string(Buff, Len);
  } while(Len > 0);
  pConfig = json::parse(Read, NULL, false);
  if(pConfig.is_discarded()) pConfig = json::object();
  pthread_mutex_unlock(&pMutex);
}
ConfigManager::~ConfigManager() {
  pthread_mutex_lock(&pMutex);
  if(pFile != -1) close(pFile);
  pFile = -1;
  pthread_mutex_unlock(&pMutex);
}
bool ConfigManager::WriteConfig(const string &path, const json &val) {
  pthread_mutex_lock(&pMutex);
  LogFile::Debug("Saving '%s' settings", path.c_str());
  if(pFile == -1) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pConfig[json::json_pointer(path)] = val;
  lseek(pFile, 0, SEEK_SET);
  string Write = pConfig.dump(2);
  if(write(pFile, Write.c_str(), Write.size()) < 0) {
    LogFile::Error("Can not write configuration to file (%s)", strerror(errno));
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(ftruncate(pFile, Write.size()) < 0 ) {
    LogFile::Error("Can not write configuration to file (%s)", strerror(errno));
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pthread_mutex_unlock(&pMutex);
  return true;
}
json ConfigManager::ReadConfig(const string &path) {
  pthread_mutex_lock(&pMutex);
  LogFile::Debug("Reading '%s' settings", path.c_str());
  if((pFile == -1) || (pConfig.is_object() == false)) {
    pthread_mutex_unlock(&pMutex);
    return nullptr;
  }
  json Result = pConfig[json::json_pointer(path)];
  pthread_mutex_unlock(&pMutex);
  return Result;
}
// ---------------------------------------------------------------------------------------------------------------------