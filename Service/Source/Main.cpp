/* ================================== zMeter ==================================
 * File:     Main.cpp
 * Author:   Pedro Sanchez (3 de marzo de 2015)
 * Modified: Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#include <signal.h>
#include <string.h>
#include <errno.h>
#include "Tools.h"
#include "HTTPServer.h"
#include "DeviceInterface.h"
#include "NetworkManager.h"
//#include "Capture.h"
static bool Terminate = false;
void SignalHandler(int signal) {
  if((signal == SIGINT) || (signal == SIGTERM)) Terminate = true;
}
int main(int argc, char** argv) {
  for(int i = 20; i < 20; i++) {
    printf("Esperando al depurador (%d)\n", i);
    fflush(stdout);
    sleep(1);
  }
//  NetworkManager *Networks = new NetworkManager();
//  Networks->SendGetDevices();
//  Tools::GetJSONNetworks();
//  return 0;
//  NetworkManager::SetJSONNetworkConfig(NULL);
//  NetworkManager::GetJSONNetworks();
//  return 0;
  
  char *ConfigFile = (char*)"/etc/openZmeter.conf";
  bool  Daemonize  = false;
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
  if(Daemonize == true) Tools::Daemonize();
  int Prio = sched_get_priority_max(SCHED_RR);
  if(Prio == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    struct sched_param Param;
    Param.sched_priority = Prio;
    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) Tools::LogMsg("WARN", "Cant raise process priority (%s:%d -> %s)\n", __func__, __LINE__, strerror(errno));
  }
  Tools::RegisterHandlers(SignalHandler);
  HTTPServer::RegisterCallback((char*)"/getSysInfo",      Tools::GetSysInfo);
  HTTPServer::RegisterCallback((char*)"/getAnalyzers",    DeviceInterface::GetAnalyzersCallback);
  HTTPServer::RegisterCallback((char*)"/getSeries",       DeviceInterface::GetSeriesCallback);
  HTTPServer::RegisterCallback((char*)"/getSeriesAggreg", DeviceInterface::GetSeriesAggregCallback);
  HTTPServer::RegisterCallback((char*)"/getSeriesNow",    DeviceInterface::GetSeriesNowCallback);
  HTTPServer::RegisterCallback((char*)"/getSeriesRange",  DeviceInterface::GetSeriesRangeCallback);
  
  HTTPServer::RegisterMapping((char*)"/",                     (char*)"Dashboard.html");
  HTTPServer::RegisterMapping((char*)"/Dashboard",            (char*)"Dashboard.html");
  HTTPServer::RegisterMapping((char*)"/Analysis/Frequency",   (char*)"Analysis_Frequency.html");
  HTTPServer::RegisterMapping((char*)"/Analysis/Voltage",     (char*)"Analysis_Voltage.html");
  HTTPServer::RegisterMapping((char*)"/Analysis/Current",     (char*)"Analysis_Current.html");
  HTTPServer::RegisterMapping((char*)"/Analysis/Power",       (char*)"Analysis_Power.html");
  HTTPServer::RegisterMapping((char*)"/Analysis/Energy",      (char*)"Analysis_Energy.html");
  HTTPServer::RegisterMapping((char*)"/Events/ITIC",          (char*)"Events_ITIC.html");
  HTTPServer::RegisterMapping((char*)"/Events/Disturbances",  (char*)"Events_Disturbances.html");
  HTTPServer::RegisterMapping((char*)"/Events/EN50160",       (char*)"Events_EN50160.html");
  HTTPServer::RegisterMapping((char*)"/Stats",                (char*)"Stats.html");
  HTTPServer::RegisterMapping((char*)"/Settings",             (char*)"Settings.html");
  HTTPServer::RegisterMapping((char*)"/Help",                 (char*)"Help.html");



  if(Tools::ReadConfig(ConfigFile) == false) return EXIT_FAILURE;
  
//  if(Tools::CreateLogFile() == false) return EXIT_FAILURE;
  //CaptureInterface **Interfaces = (CaptureInterface**)calloc(sizeof(CaptureInterface*), Tools::Devices.Len);
//  for(uint i = 0; i < Tools::Devices.Len; i++) {
//    if(strcmp(Tools::Devices.Items[i].Type, "Mono") == 0) {
//      Tools::Devices.Items[i].Instance = new CaptureMono(Tools::Devices.Items[i].Serial, Tools::Devices.Items[i].VoltageGain, Tools::Devices.Items[i].VoltageOffset, 
//                                                         Tools::Devices.Items[i].CurrentGain, Tools::Devices.Items[i].CurrentOffset, i);
//    } else if(strcmp(Tools::Devices.Items[i].Type, "VirtualMono") == 0) {
//      Tools::Devices.Items[i].Instance = new CaptureVirtual(true, 1.0, i);
//    }
//  }
//  if(HTTPServer::Start() == false) return EXIT_FAILURE;
    
  //  uint32_t LastPrice;
//  if(DB_Start(&LastPrice) == true) {
//    if(Analyzer_Start() == false) goto Stop;
//    DB_Started = true;
//  }

//  if(RequestServer_Start(Config.Global.Listen) == false) goto Stop;
//  Devices::Start();
//  HTTPServer::Start();

//  Events_Init();
//  Prices_Start(LastPrice);
  int Iter = 0;
  while(!Terminate) {
    usleep(1000000);
    if((Iter % 10) == 0) {
      if(NetworkManager::GetConnectivityStatus() < 2) NetworkManager::Reset();
    }
    if((Iter % 100) == 0) {
      HTTPServer::RemoveExpiredSessions();
    
    }
    Iter++;
    
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
//  HTTPServer::stop();
  //  Prices_Stop();
//  Analyzer_Stop();
//  RequestServer_Stop();
//  Tools::RemovePidFile();
  return EXIT_SUCCESS;
}

