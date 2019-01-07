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
#include "NTP.h"
static bool Terminate = false;
void SignalHandler(int signal) {
  if((signal == SIGINT) || (signal == SIGTERM)) Terminate = true;
}
bool GetNewsCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  uint64_t LastLogin;
  if(Tools::JSONUInt64(reader, "LastLogin", &LastLogin) == false) {
    *postdata  = (char*)"'LastLogin' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  char Query[1024];  
  sprintf(Query, "SELECT row_to_json(Result) AS result FROM (SELECT COALESCE(ARRAY_AGG(header ORDER BY newstime ASC), '{}') AS \"Headers\", COALESCE(ARRAY_AGG(contents ORDER BY newstime ASC), '{}') AS \"Contents\", COALESCE(ARRAY_AGG(newstime ORDER BY newstime ASC), '{}') AS \"Timestamp\" FROM news WHERE newstime >= %" PRIu64 ") AS Result", LastLogin);
  *postdata = db->ExecSentenceJSON(Query, resultLen);
  return true;  
}
int main(int argc, char** argv) {
  for(int i = 20; i < 20; i++) {
    printf("Esperando al depurador (%d)\n", i);
    fflush(stdout);
    sleep(1);
  }
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
//  NTP(NULL);
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
  HTTPServer::RegisterCallback("/getSysInfo",          Tools::GetSysInfo);
  HTTPServer::RegisterCallback("/getAnalyzers",        DeviceInterface::GetAnalyzersCallback);
  HTTPServer::RegisterCallback("/getSeries",           DeviceInterface::GetSeriesCallback);
  HTTPServer::RegisterCallback("/getSeriesAggreg",     DeviceInterface::GetSeriesAggregCallback);
  HTTPServer::RegisterCallback("/getSeriesNow",        DeviceInterface::GetSeriesNowCallback);
  HTTPServer::RegisterCallback("/getSeriesRange",      DeviceInterface::GetSeriesRangeCallback);
  HTTPServer::RegisterCallback("/getEvents",           DeviceInterface::GetEventsCallback);
  HTTPServer::RegisterCallback("/getPrices",           DeviceInterface::GetPricesCallback);
  HTTPServer::RegisterCallback("/getNetworks",         NetworkManager::GetNetworksCallback);
  HTTPServer::RegisterCallback("/getNews",             GetNewsCallback);
  
  HTTPServer::RegisterDefault("/Dashboard");
  HTTPServer::RegisterMapping("/Dashboard",            "/Dashboard.html");
  HTTPServer::RegisterMapping("/Analysis/Frequency",   "/Analysis_Frequency.html");
  HTTPServer::RegisterMapping("/Analysis/Voltage",     "/Analysis_Voltage.html");
  HTTPServer::RegisterMapping("/Analysis/Current",     "/Analysis_Current.html");
  HTTPServer::RegisterMapping("/Analysis/Power",       "/Analysis_Power.html");
  HTTPServer::RegisterMapping("/Analysis/Energy",      "/Analysis_Energy.html");
  HTTPServer::RegisterMapping("/Events/ITIC",          "/Events_ITIC.html");
  HTTPServer::RegisterMapping("/Events/Disturbances",  "/Events_Disturbances.html");
  HTTPServer::RegisterMapping("/Events/EN50160",       "/Events_EN50160.html");
  HTTPServer::RegisterMapping("/Stats/Costs",          "/Stats_Costs.html");
  HTTPServer::RegisterMapping("/Stats/Heatmap",        "/Stats_Heatmap.html");
  HTTPServer::RegisterMapping("/Stats/Distribution",   "/Stats_Distribution.html");
  HTTPServer::RegisterMapping("/Settings",             "/Settings.html");
  HTTPServer::RegisterMapping("/Help",                 "/Help.html");

  if(Tools::ReadConfig(ConfigFile) == false) return EXIT_FAILURE;

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

