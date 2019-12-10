/* ================================== zMeter ==================================
 * File:     Main.cpp
 * Author:   Pedro Sanchez (3 de marzo de 2015)
 * Modified: Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cctz/time_zone.h>
#include "Tools.h"
#include "HTTPServer.h"
#include "DeviceManager.h"
#include "SysSupport.h"
//---------------------------------------------------------------------------------------------------------------------
using namespace cctz;
static bool Terminate = false;
//---------------------------------------------------------------------------------------------------------------------
void SignalHandler(int signal) {
  if((signal == SIGINT) || (signal == SIGTERM)) Terminate = true;
}
void RunDaemon() {
  LOGFile::Info("Move to background task\n");
  pid_t ProcID = fork();
  if(ProcID < 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    exit(1);
  }
  if(ProcID > 0) exit(0);
  umask(0);
  pid_t SID = setsid();
  if(SID < 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    exit(1);
  }
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}
int main(int argc, char** argv) {
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
    } else if(strcmp(argv[i], "--daemon") == 0) {
      RunDaemon();
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
  /* Create database tables */
  LOGFile::Debug("Creating database tables\n");
  Database::EnablePool();
  Database DB("public");
  if(DB.ExecResource("SQL/Database.sql") == false) LOGFile::Error("Database is damaged, manual repair is nedded\n");
  /* Create system manager */
  SysSupport SystemManager;
  /* Create devices interface */
  DeviceManager DeviceManager;
  /* Create WEB service */
  HTTPServer HTTPServer;
  uint32_t Iter = 0;
  while(!Terminate) {
    SysSupport::PeriodicTask();
    if((Iter % 3600) == 0) HTTPServer.RemoveExpiredSessions(DB);
    if((Iter % 600) == 0) Database::RemoveExpiredSessions();
    Iter++;
    usleep(1000000);
  }
  Database::DisablePool();
  return EXIT_SUCCESS;
}