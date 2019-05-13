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
#include "Tools.h"
#include "HTTPServer.h"
#include "DeviceInterface.h"
#include "NetworkManager.h"
#include "NTP.h"
#include "Bluetooth.h"
//---------------------------------------------------------------------------------------------------------------------
void GetNewsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  uint64_t Time = 0;
  if(Params["NewsTime"].is_number()) Time = Params["NewsTime"];
  if(!Params["Action"].is_string()) {
    req.ExecSentenceAsyncResult("SELECT COALESCE((SELECT jsonb_build_object('NewsTime', newstime, 'Header', header, 'Contents', contents) FROM news WHERE newstime = " + req.Bind(Time) + "), 'null') AS result");
  } else if(Params["Action"] == "Before") {
    req.ExecSentenceAsyncResult("SELECT COALESCE((SELECT jsonb_build_object('NewsTime', newstime, 'Header', header, 'Contents', contents) FROM news WHERE newstime < " + req.Bind(Time) + " ORDER BY newstime DESC LIMIT 1), 'null') AS result");    
  } else if(Params["Action"] == "After") {
    req.ExecSentenceAsyncResult("SELECT COALESCE((SELECT jsonb_build_object('NewsTime', newstime, 'Header', header, 'Contents', contents) FROM news WHERE newstime > " + req.Bind(Time) + " ORDER BY newstime ASC LIMIT 1), 'null') AS result");
  } else {
    return req.FromString("'Action' must be 'After' or 'Before'", MHD_HTTP_BAD_REQUEST);
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
  signal(SIGILL, SIG_IGN);
  string ConfigFileName = "/etc/openZmeter.conf";
  bool   Daemonize      = false;
  bool   Debug          = false;
  for(int i = 1; i != argc; i++) {
    if(memcmp(argv[i], "--config=", 9) == 0) {
      ConfigFileName = &argv[i][9];
    } else if(strcmp(argv[i], "--debug") == 0) {
      Debug = true;
    } else if(strcmp(argv[i], "--daemon") == 0) {
      Daemonize = true;
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
  if(Daemonize == true) RunDaemon();
  /* Create resource loader */
  ResourceLoader ResourceLoader;
  /* Create config manager */
  ConfigManager ConfigManager(ConfigFileName);
  /* Create bluetooth manager */
  Bluetooth BluetoothManager;
  /* Create PID File */
  PIDFile PIDFile;
  /* Create LOG file */
  LOGFile LOGFile(Debug, !Daemonize);
  /* Create database */
  if(Database::Config() == false) return EXIT_FAILURE;

  /* Create embedded NTP client */
//  json NTPConfig = json::object();
//  if(Config["HTTPServer"].is_object()) NTPConfig = Config["HTTPServer"];
  //  if((json_object_object_get_ex(pConfig, "NTP", &Find) != true) || (json_object_get_type(Find) != json_type_object)) json_object_object_add(pConfig, "NTP", json_object_new_object());
//  if((json_object_object_get_ex(pConfig, "NTP", &Find) != true) || (json_object_get_type(Find) != json_type_object) || (NTP::Config(Find) == false)) {
//    pthread_mutex_unlock(&pConfigMutex);
//    LogMsg("ERROR", "Can not create NTP client\n");
//    return false;
//  }

  /* Create devices interface */
  DeviceManager DeviceManager;
  
  /* Create WEB service */
  HTTPServer HTTPServer;
  if(HTTPServer.Config() == false) return EXIT_FAILURE;
//  HTTPServer.RegisterCallback("/getSysInfo",          Tools::GetSysInfo);
  HTTPServer::RegisterCallback("/getNetworks",         NetworkManager::GetNetworksCallback);
  HTTPServer::RegisterCallback("/getNews",             GetNewsCallback);
  HTTPServer::RegisterCallback("/getCountries",        GetCountriesCallback);  
  HTTPServer::RegisterCallback("/getUser",             GetUserCallback);
  HTTPServer::RegisterCallback("/setUser",             SetUserCallback);
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  int Prio = sched_get_priority_max(SCHED_RR);
  if(Prio == -1) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    sched_param Param {.sched_priority = Prio };
    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) LOGFile::Warning("Cant raise process priority (%s:%d -> %s)\n", __func__, __LINE__, strerror(errno));
  }

  int Iter = 0;
  while(!Terminate) {
    usleep(1000000);
    if((Iter % 10) == 0) {
//      if(NetworkManager::GetConnectivityStatus() < 2) NetworkManager::Reset();
    }
    if((Iter % 128) == 0) HTTPServer.RemoveExpiredSessions();
    Iter++;
  }
  return EXIT_SUCCESS;
}