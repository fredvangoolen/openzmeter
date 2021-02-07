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
#include "LogFile.h"
#include "PidFile.h"
#include "HTTPServer.h"
#include "TelegramManager.h"
#include "DeviceManager.h"
#include "UserManager.h"
#include "NTPManager.h"
#include "WifiManager.h"
#include "EthernetManager.h"
#include "BluetoothManager.h"
#include "AnalyzerManager.h"
#include "TariffManager.h"
#include "ConfigFile.h"
#include "UpdateManager.h"
#include "SystemManager.h"
#include "ModbusManager.h"
#include "DatabasePool.h"
#include "MessageLoop.h"
#include "ESIOS/ESIOSManager.h"
//---------------------------------------------------------------------------------------------------------------------
volatile bool Terminate = false;
//---------------------------------------------------------------------------------------------------------------------
void SignalHandler(int signal) {
  LogFile::Info("Shutting down. Please wait while all task ends...");
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
      LogFile::EnableDebug();
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
  Worker::SetThreadProperties(pthread_self(), "Main", Worker::NORMAL);
  ConfigFile       ConfigFile(ConfigFileName);     // Load config file
  PidFile          PIDFile;                        // Create PID file
  LogFile          LogFile;                        // Create LOG file
  DatabasePool     DBPool;                         // Init database pool
  HTTPServer       HTTPServer;                     // Init WEB service
  MessageLoop      Loop;                           // Create application message loop
  SystemManager    Sys;                            // System status and actions

  BluetoothManager BT;                             // Create bluetooth manager
  WifiManager      Wifi;                           // Create wifi manager
  EthernetManager  Ethernet;                       // Create ethernet manager
  UpdateManager    Update;                         // Create update manager
  NTPManager       NTP;                            // Create NTP manager
  UserManager      User;                           // Create user manager
  TariffManager    TariffManager;                  // Create tariffs interface
  ESIOSManager     Esios;                          // Create ESIOS manager
  AnalyzerManager  AnalyzerManager;                // Create analyzers interface
  DeviceManager    DeviceManager;                  // Create devices interface
  TelegramManager  TelegramBoot;                   // Create Telegram manager
  ModbusManager    Modbus;                         // Create ModbusTCP

  while(!Terminate) MessageLoop::HandleEvents();
  return EXIT_SUCCESS;
}
//---------------------------------------------------------------------------------------------------------------------