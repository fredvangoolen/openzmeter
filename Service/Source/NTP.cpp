#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include "NTP.h"
#include "HTTPServer.h"
//======================================================================================================================
bool            NTP::pTerminate = false;
pthread_t       NTP::pThread = 0;
bool            NTP::pEnabled = false;
string          NTP::pServer;
pthread_mutex_t NTP::pMutex = PTHREAD_MUTEX_INITIALIZER;
//======================================================================================================================
bool NTP::Config(json_object *config) {
  if(config == NULL) return false;
  json_object_object_foreach(config, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Server") == 0)) {
      pServer = strdup((char*)json_object_get_string(JSON_Val));
    } else if((JSON_Type == json_type_boolean) && (strcmp(JSON_Key, "Enabled") == 0)) {
      pEnabled = json_object_get_boolean(JSON_Val);
    }
  }  
  HTTPServer::RegisterCallback("/getTimeSettings", GetTimeSettingsCallback);
  HTTPServer::RegisterCallback("/setTimeSettings", SetTimeSettingsCallback);
  if(pthread_create(&pThread, NULL, NTP_Thread, NULL) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false;
  }
  pthread_setname_np(pThread, "NTP_Client");
  return true;
}
void *NTP::NTP_Thread(void *args) {
  NTP *This = (NTP*)args;
  int Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(Socket < 0) {
    Tools::LogMsg("ERROR", "NTP Client can not open socket\n");
    return NULL;
  }
  NTP_Packet Packet;
  timeval Timeout;
  Timeout.tv_sec  = 0;
  Timeout.tv_usec = 100000;
  if(setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, &Timeout, sizeof(Timeout)) < 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  while(!This->pTerminate) {
    hostent *Server = NULL;
    pthread_mutex_lock(&pMutex);
    if(pEnabled == false) goto Release;
    memset(&Packet, 0, sizeof(NTP_Packet));
    Packet.li_vn_mode = 0x1B;
    Server = gethostbyname(pServer.c_str());
    if(Server == NULL) {
      Tools::LogMsg("ERROR", "NTP server can not be resolved\n");
      goto Release;
    }
    sockaddr_in ServerAddr;
    memset(&ServerAddr, 0, sizeof(ServerAddr));
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = *((unsigned long*) Server->h_addr);
    ServerAddr.sin_port = htons(123);
    if(sendto(Socket, &Packet, sizeof(NTP_Packet), 0, (sockaddr *)&ServerAddr, sizeof(ServerAddr)) < 0) {
      Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      goto Release;
    }
    if(read(Socket, &Packet, sizeof(NTP_Packet)) != sizeof(NTP_Packet)) {
      Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      goto Release;
    }
    Packet.txTm_s = ntohl(Packet.txTm_s); // Time-stamp seconds.
    Packet.txTm_f = ntohl(Packet.txTm_f); // Time-stamp fraction of a second.
    timeval TimeVal;
    TimeVal.tv_sec  = Packet.txTm_s - 2208988800ull;
    if(Packet.txTm_f != 0) TimeVal.tv_usec = 0xFFFFFFFF / (Packet.txTm_f / 1000);
    if(settimeofday(&TimeVal, NULL) != 0) {
      Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      goto Release;
    }
    fflush(stdout);
  Release :
    pthread_mutex_unlock(&pMutex);
    sleep(10);
  }
  return NULL;
}
void NTP::GetTimeSettingsCallback(HTTPRequest &req) {
  json Result;

  //  *writer = json_object_new_object();
//  FILE *File = fopen("/usr/share/zoneinfo/zone.tab", "r");
//  if(File != NULL) {
//    json_object *Zones = json_object_new_array();
//    char  *Line = NULL;
//    size_t Len  = 0;
//    while(getline(&Line, &Len, File) != -1) {
//      if(Line[0] == '#') continue;
//      char  *Tmp  = Line;
//      strtok_r(Tmp, "\t", &Tmp);
//      strtok_r(Tmp, "\t", &Tmp);
//      char *Result = strtok_r(Tmp, "\t", &Tmp);
//      while((strlen(Result) > 0) && ((Result[strlen(Result) - 1] == '\n') || (Result[strlen(Result) - 1] == '\r'))) Result[strlen(Result) - 1] = 0;
//      if(Result) json_object_array_add(Zones, json_object_new_string(Result));
//    }
//    if(Line != NULL) free(Line);
//    fclose(File);
//    json_object_object_add(*writer, "TimeZones", Zones);
//  }
//  File = fopen("/etc/timezone", "r");
//  if(File != NULL) {
//    char  *Line = NULL;
//    size_t Len  = 0;
//    if(getline(&Line, &Len, File) != -1) {
//      while((strlen(Line) > 0) && ((Line[strlen(Line) - 1] == '\n') || (Line[strlen(Line) - 1] == '\r'))) Line[strlen(Line) - 1] = 0;
//      json_object_object_add(*writer, "TimeZone", json_object_new_string(Line));
//    }
//    if(Line != NULL) free(Line);
//    fclose(File);
//  }
  Result["NTP_Enabled"] = pEnabled;
  Result["NTP_Server"] = pServer;
  Result["Time"] = Tools::GetTime();
  req.FromJSON(Result);
}
void NTP::SetTimeSettingsCallback(HTTPRequest &req) {
//  bool Error = false;
//  if(user == "admin") {
//    *postdata  = (char*)"Only admin can change time settings";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  pthread_mutex_lock(&pMutex);
//  json_object_object_foreach(reader, JSON_Key, JSON_Val) {
//    enum json_type JSON_Type = json_object_get_type(JSON_Val);
//    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "NTP_Server") == 0)) {
//      if(pServer) free(pServer);
//      pServer = strdup((char*)json_object_get_string(JSON_Val));
//    } else if((JSON_Type == json_type_boolean) && (strcmp(JSON_Key, "NTP_Enabled") == 0)) {
//      pEnabled = json_object_get_boolean(JSON_Val);
//    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "TimeZone") == 0)) {
//      FILE *File = fopen("/etc/timezone", "w");
//      if(File != NULL) {
//        fprintf(File, "%s\n", (char*)json_object_get_string(JSON_Val));
//        fclose(File);
//      } else {
//        Error = true;
//        Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//      }
//      if(unlink("/etc/localtime") != 0) {
//        char Zone[256];
//        snprintf(Zone, sizeof(Zone), "/usr/share/zoneinfo/%s", (char*)json_object_get_string(JSON_Val));
//        if(symlink(Zone, "/etc/localtime") != 0) {
//          Error = true;
//          Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//        }
//      } else {
//        Error = true;
//        Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//      }
//    } else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Time") == 0)) {
//      uint64_t Time = json_object_get_int64(JSON_Val);
//      timeval TimeVal;
//      TimeVal.tv_sec  = Time / 1000;
//      TimeVal.tv_usec = Time % 1000;
//      if(settimeofday(&TimeVal, NULL) != 0) {
//        Error = true;
//        Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//      }
//    }
//  }
//  pthread_mutex_unlock(&pMutex);
//  json_object *NTPConfig = json_object_new_object();
//  json_object_object_add(NTPConfig, "Server", json_object_new_string((pServer == NULL) ? "" : pServer));
//  json_object_object_add(NTPConfig, "Enabled", json_object_new_boolean(pEnabled));
//  json_object_object_add(Tools::GetConfigObject(), "NTP", NTPConfig);
//  if((Tools::SaveConfig() == false) || (Error == true)) {
//    *postdata  = (char*)"Can not save settings";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  *writer = json_object_new_object();
//  json_object_object_add(*writer, "Result", json_object_new_boolean(true));
//  *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
}