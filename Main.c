/* ================================== zMeter ==================================
 * File:     main.c
 * Author:   Pedro Sanchez (3 de marzo de 2015)
 * Modified: Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "Main.h"

bool CloseRequest = false;
void SignalHandler(int signal) {
  if((signal == SIGINT) || (signal == SIGTERM)) CloseRequest = true;
}
void LogMsg(const char *level, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  time_t Now;
  struct tm *TimeStamp;
  char DateStr[80];
  flockfile(stdout);
  if(strcmp(level, "ERROR") == 0) {
    printf("\033[1;31m");
  } else if(strcmp(level, "DEBUG") == 0) {
    printf("\033[1;33m");
  } else if(strcmp(level, "INFO") == 0) {
    printf("\033[1;34m");
  } else {
    return;
  }
  time(&Now);
  TimeStamp = localtime(&Now);
  strftime(DateStr, 80, "[%d/%m/%Y - %X] ", TimeStamp);
  printf("%s", DateStr);
  printf("\033[0m");
  vprintf(msg, args);
  fflush(stdout);
  funlockfile(stdout);
  va_end(args);
}
bool CreatePidFile(long pid) {
  char PID_Number[64];
  int Len = sprintf(PID_Number, "%d", getpid());
  remove("/var/run/openZmeter.pid");
  int File = open("/var/run/openZmeter.pid", O_CREAT | O_WRONLY);
  if(File < 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false;
  }
  write(File, PID_Number, Len);
  close(File);
  return true;
}
int RemovePidFile() {
 return remove("/var/run/openZmeter.pid");
}
int main(int argc, char** argv) {
  char *ConfigFile = "/etc/openZmeter.conf";
  bool  Daemonize  = false;
  printf("=================================================================================\n");
  printf("           ** openZmeter - Electric quality analysis and metering **             \n");
  printf("=================================================================================\n");
  for(int i = 1; i != argc; i++) {
    if(memcmp(argv[i], "--config=", 9) == 0) {
      ConfigFile = &argv[i][9];
    } else if(strcmp(argv[i], "--debug") == 0) {
      Config.Debug = true;
    } else if(strcmp(argv[i], "--daemon") == 0) {
      Daemonize  = true;
    } else if(strcmp(argv[i], "--help") == 0) {
      printf("--help           - Shows this screen");
      printf("--config=<file>  - Set config file\n");
      printf("--debug          - Enable debug messages\n");
      printf("--daemon         - Detach from console and run as service ");
      return 0;
    } else {
      printf("Option %s not recogniced. Use --help to show valid options.", argv[i]);
    }
  }
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);
  setpriority(PRIO_PROCESS, 0, -20);
  if(Config_Init(ConfigFile) == false) goto Stop;

  char ConnectionString[512];
  sprintf(ConnectionString, "host=%s port=%d user=%s password=%s dbname=%s", Config.DB.Host, Config.DB.Port, Config.DB.User, Config.DB.Password, Config.DB.Database);
  uint32_t LastPrice = DB_Start(ConnectionString, Config.DB.QueueLen);
  if(LastPrice == 0) goto Stop;
  Events_Init();
  if(RequestServer_Start(Config.Global.Listen) == false) goto Stop;
//  Prices_Start(LasPriceUpdate);
  Adquisition_Start(ADQUISITION_DEVICE, ADQUISITION_BUFFERSIZE, 50.0);

  while(!CloseRequest) {
    Work_t *Work = Adquisition_GetWork();
    if(Work->SequenceID == 0) {
      free(Work);
      continue;
    }
    uint64_t Start, End;
    struct timespec Time;
    clock_gettime(CLOCK_MONOTONIC, &Time);
    Start = Time.tv_sec * 1000000 + Time.tv_nsec / 1000;
    
    Params_t *Params = Analyzer_GetParams(Work);
    if((Params->SequenceID) % 5 == 0) Adquisition_SetLEDs(Params->RMS_V, Params->RMS_I);
    if(Params) DB_Save(PARAMS_10C, Params, true);

    clock_gettime(CLOCK_MONOTONIC, &Time);
    End = Time.tv_sec * 1000000 + Time.tv_nsec / 1000;
    LogMsg("DEBUG", "Processing block took %.1f mSec\n", (End - Start) / 1000.0);
  }
Stop:
//  PriceServer_Stop();
//  DB_Stop();
  Adquisition_Stop();
  RequestServer_Stop();
  return EXIT_SUCCESS;
}

