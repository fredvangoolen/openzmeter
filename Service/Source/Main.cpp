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
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <cctz/time_zone.h>
#include "Tools.h"
#include "HTTPServer.h"
#include "Telegram.h"
#include "Devices/DeviceManager.h"
#include "SysSupport.h"
#include "Tariffs/ESIOS.h"
#include "MODBus.h"
//---------------------------------------------------------------------------------------------------------------------
using namespace cctz;
volatile bool Terminate = false;
//---------------------------------------------------------------------------------------------------------------------
void SignalHandler(int signal) {
  LOGFile::Info("Shutting down. Please wait while all task ends...\n");
  Terminate = true;
}
int main(int argc, char** argv) {
  Tools::DisableEcho();
  setbuf(stdout, NULL);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGILL, SIG_IGN);
  signal(SIGALRM, SIG_IGN);
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  string ConfigFileName = "/etc/openZmeter.conf";
  for(int i = 1; i != argc; i++) {
    if(memcmp(argv[i], "--config=", 9) == 0) {
      ConfigFileName = &argv[i][9];
    } else if(strcmp(argv[i], "--debug") == 0) {
      LOGFile::EnableDebug();
    } else if(strcmp(argv[i], "--daemon") == 0) {
      Tools::RunDaemon();
    } else if(strcmp(argv[i], "--help") == 0) {
      printf("--help           - Shows this screen");
      printf("--config=<file>  - Set config file\n");
      printf("--daemon         - Detach from console and run as service ");
      return 0;
    } else {
      printf("Option %s not recogniced. Use --help to show valid options.\n\n", argv[i]);
    }
  }
  /* Load config file */
  ConfigManager ConfigManager(ConfigFileName);
  /* Create PID File */
  PIDFile PIDFile;
  /* Create LOG file */
  LOGFile LOGFile;
  /* Create system manager */
  SysSupport SystemManager;
  /* Create database tables */
  DatabasePool DBPool;
  /* Create WEB service */
  HTTPServer HTTPServer;
  /* Create Telegram manager */
  Telegram TG;
  /* Create devices interface */
  DeviceManager DeviceManager;
  /* Create ESIOS */
  ESIOS Esios;
  /* Create ModbusTCP */
  ModbusTCP Modbus;

  pthread_setname_np(pthread_self(), "openZmeter");
  nice(15);

  uint32_t Iter = 0;
  while(!Terminate) {
    SysSupport::PeriodicTask();
    if((Iter % 80) == 0)  HTTPServer::PeriodicTasks();
    if((Iter % 80) == 20) DeviceManager::PeriodicTasks();
    if((Iter % 80) == 40) DatabasePool::PeriodicTasks();
    if((Iter % 80) == 60) Telegram::PeriodicTasks();
    Iter++;
    usleep(1000000);
  }
  return EXIT_SUCCESS;
}