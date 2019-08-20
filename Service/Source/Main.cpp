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
//---------------------------------------------------------------------------------------------------------------------
void GetNewsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(Params["NewsTime"].is_number()) {
    uint64_t Time = Params["NewsTime"];
    req.ExecSentenceAsyncResult("SELECT COALESCE((SELECT json_build_object('NewsTime', newstime, 'Header', header, 'Contents', contents) FROM news WHERE newstime = " + req.Bind(Time) + "), 'null') AS result");
  } else {
    req.ExecSentenceAsyncResult("SELECT json_build_object('Version', '" __BUILD__ "', 'News', array_agg(newstime ORDER BY newstime ASC)) AS result from news");
  }
  if(req.FecthRow() == false) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("result"), MHD_HTTP_OK);
}
void GetUserCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  req.ExecSentenceAsyncResult("SELECT json_build_object('Name', name, 'Address', address, 'Company', company, 'Photo', photo, 'EMail', email, 'TelegramID', telegram, 'Country_Code', country_code) AS result FROM users WHERE username = " + req.Bind(req.UserName()));
  if(req.FecthRow() == false) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("result"), MHD_HTTP_OK);
}
void SetUserCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //req.ExecSentenceAsyncResult("UPDATE users SET name = ", 'Address', address, 'Company', company, 'Photo', photo, 'EMail', email, 'TelegramID', telegram, 'Country_Code', country_code) AS result FROM users WHERE username = " + req.Bind(req.UserName()));
  if(req.FecthRow() == false) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("result"), MHD_HTTP_OK);
}
void GetCountriesCallback(HTTPRequest &req) {
  req.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('Country_Code', country_code, 'Name', country, 'Currency', currency, 'TimeZone', timezone))), '[]') as result FROM country");
  if(req.FecthRow() == false) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("result"), MHD_HTTP_OK);
}
//---------------------------------------------------------------------------------------------------------------------
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
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);
  
//  for(auto it = cctz::time_zone_map.begin(); it != cctz::time_zone_map.end(); it++) {
//    printf("%s\n", it->first.c_str());
//  }
  
  
  
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
  {
    Database DB("public");
    if(DB.ExecResource("SQL/Database.sql") == false) return EXIT_FAILURE;
  }
  /* Create system manager */
  System SystemManager;

  /* Create devices interface */
  DeviceManager DeviceManager;
  
  /* Create WEB service */
  HTTPServer HTTPServer;
//  if(HTTPServer.Config() == false) return EXIT_FAILURE;
//  HTTPServer.RegisterCallback("/getSysInfo",          Tools::GetSysInfo);
//  HTTPServer::RegisterCallback("/getNetworks",         NetworkManager::GetNetworksCallback);
  HTTPServer::RegisterCallback("getNews",             GetNewsCallback);
  HTTPServer::RegisterCallback("getCountries",        GetCountriesCallback);  
  HTTPServer::RegisterCallback("getUser",             GetUserCallback);
  HTTPServer::RegisterCallback("setUser",             SetUserCallback);


//  int Prio = sched_get_priority_max(SCHED_RR);
//  if(Prio == -1) {
//    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//  } else {
//    sched_param Param {.sched_priority = Prio };
//    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) LOGFile::Warning("Cant raise process priority (%s:%d -> %s)\n", __func__, __LINE__, strerror(errno));
//  }

  uint32_t Iter = 0;
  while(!Terminate) {
    SystemManager.PeriodicTask();
    if((Iter % 3600) == 0) HTTPServer.RemoveExpiredSessions();
    Iter++;
    usleep(1000000);
  }
  return EXIT_SUCCESS;
}