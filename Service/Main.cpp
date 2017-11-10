/* ================================== zMeter ==================================
 * File:     main.cpp
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


#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "Main.h"
#include "Tools.h"
#include "HTTPServer.h"
#include "CaptureInterface.h"
#include "CaptureMono/CaptureMono.h"
#include "CaptureVirtual/CaptureVirtual.h"

bool  Terminate  = false;
bool  Daemonize  = false;
bool  DB_Started = false;

void SignalHandler(int signal) {
  if((signal == SIGINT) || (signal == SIGTERM)) Terminate = true;
}
int main(int argc, char** argv) {
  char *ConfigFile = (char*)"/etc/openZmeter.ini";
  for(int i = 1; i != argc; i++) {
    if(memcmp(argv[i], "--config=", 9) == 0) {
      ConfigFile = &argv[i][9];
    } else if(strcmp(argv[i], "--debug") == 0) {
      Tools::Debug = true;
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
  if(Daemonize == true) Tools::Daemonize();
  if(Tools::ReadConfig(ConfigFile) == false) return EXIT_FAILURE;
  Tools::CreatePidFile();
  Tools::CreateLogFile();
  HTTPServer::Start();
  CaptureInterface **Interfaces = (CaptureInterface**)calloc(sizeof(CaptureInterface*), Tools::Devices.Len);
  for(int i = 0; i < Tools::Devices.Len; i++) {
    if(strcmp(Tools::Devices.Items[i].Type, "Mono") == 0) {
      Interfaces[i] = new CaptureMono(Tools::Devices.Items[i].Serial, Tools::Devices.Items[i].ID,
                                      Tools::Devices.Items[i].VoltageGain, Tools::Devices.Items[i].VoltageOffset, Tools::Devices.Items[i].CurrentGain, Tools::Devices.Items[i].CurrentOffset,
                                      128*1024, CaptureInterface::FREQ_50);
    } else if(strcmp(Tools::Devices.Items[i].Type, "VirtualMono") == 0) {
      Interfaces[i] = new CaptureVirtual(15625, true, 230, 1.0, CaptureInterface::FREQ_50);  
    }
  }
    
  //  uint32_t LastPrice;
//  if(DB_Start(&LastPrice) == true) {
//    if(Analyzer_Start() == false) goto Stop;
//    DB_Started = true;
//  }

//  if(RequestServer_Start(Config.Global.Listen) == false) goto Stop;

//  Events_Init();
//  Prices_Start(LastPrice);
  while(!Terminate) {
    usleep(1000000);
//    Work_t *Work = Adquisition_GetWork();
//    if(Work->SequenceID == 0) {
//      free(Work);
//      continue;
//    }
//    uint64_t Start, End;
//    struct timespec Time;
//    clock_gettime(CLOCK_MONOTONIC, &Time);
//    Start = Time.tv_sec * 1000000 + Time.tv_nsec / 1000;
    
//    Params_t *Params = Analyzer_GetParams(Work);
//    if((Params->SequenceID) % 5 == 0) Adquisition_SetLEDs(Params->RMS_V, Params->RMS_I);
//    if(Params) DB_Save(PARAMS_10C, Params, true);

//    clock_gettime(CLOCK_MONOTONIC, &Time);
//    End = Time.tv_sec * 1000000 + Time.tv_nsec / 1000;
//    LogMsg("DEBUG", "Processing block took %.1f mSec\n", (End - Start) / 1000.0);
  }
//Stop:
//  Prices_Stop();
//  Analyzer_Stop();
//  RequestServer_Stop();
//  Tools::RemovePidFile();
  return EXIT_SUCCESS;
}

