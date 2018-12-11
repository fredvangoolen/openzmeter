#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "NTP.h"
#include "HTTPServer.h"
bool      NTP::pTerminate = false;
pthread_t NTP::pThread = 0;
bool      NTP::pEnabled = false;
char     *NTP::pServer = NULL;
bool NTP::Config(json_object *config) {
  if(config != NULL) {
    json_object_object_foreach(config, JSON_Key, JSON_Val) {
      enum json_type JSON_Type = json_object_get_type(JSON_Val);
      if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Server") == 0)) {
        pServer = (char*)json_object_get_string(JSON_Val);
      } else if((JSON_Type == json_type_boolean) && (strcmp(JSON_Key, "Enabled") == 0)) {
        pEnabled = json_object_get_boolean(JSON_Val);
      }
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
  while(!This->pTerminate) {
    memset(&Packet, 0, sizeof(NTP_Packet));
    Packet.li_vn_mode = 0x1B;
    hostent *Server = gethostbyname("es.pool.ntp.org");
    if(Server == NULL) {
      Tools::LogMsg("ERROR", "NTP server can not be resolved\n" );
      continue;
    }
    sockaddr_in ServerAddr;
    memset(&ServerAddr, 0, sizeof(ServerAddr));
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = *((unsigned long*) Server->h_addr);
    
    ServerAddr.sin_port = htons(123);
    if(sendto(Socket, &Packet, sizeof(NTP_Packet), 0, (sockaddr *)&ServerAddr, sizeof(ServerAddr)) < 0) {
    //error( "ERROR writing to socket" );
      continue;
    }
    if(read(Socket, &Packet, sizeof(NTP_Packet)) < 0) {
      //error( "ERROR reading from socket" );
      continue;
    }
    Packet.txTm_s = ntohl( Packet.txTm_s ); // Time-stamp seconds.
    Packet.txTm_f = ntohl( Packet.txTm_f ); // Time-stamp fraction of a second.
    time_t txTm = ( time_t ) ( Packet.txTm_s - 2208988800ull );

printf( "Time: %s", ctime( ( const time_t* ) &txTm ) );
fflush(stdout);
sleep(10);
    
    
  }
  return NULL;
}
bool NTP::GetTimeSettingsCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  *writer = json_object_new_object();
  FILE *File = fopen("/usr/share/zoneinfo/zone.tab", "r");
  if(File != NULL) {
    json_object *Zones = json_object_new_array();
    char  *Line = NULL;
    size_t Len  = 0;
    while(getline(&Line, &Len, File) != -1) {
      if(Line[0] == '#') continue;
      char  *Tmp  = Line;
      strtok_r(Tmp, "\t", &Tmp);
      strtok_r(Tmp, "\t", &Tmp);
      char *Result = strtok_r(Tmp, "\t", &Tmp);
      if(Result) json_object_array_add(Zones, json_object_new_string(Result));
    }
    if(Line != NULL) free(Line);
    fclose(File);
    json_object_object_add(*writer, "TimeZones", Zones);
  }
  File = fopen("/etc/timezone", "r");
  if(File != NULL) {
    char  *Line = NULL;
    size_t Len  = 0;
    if(getline(&Line, &Len, File) != -1) {
      if((strlen(Line) > 0) && (Line[strlen(Line) - 1] == '\n')) Line[strlen(Line) - 1] = 0;
      json_object_object_add(*writer, "TimeZone", json_object_new_string(Line));
    }
    if(Line != NULL) free(Line);
    fclose(File);
  }
  json_object_object_add(*writer, "NTP_Enabled", json_object_new_boolean(pEnabled));
  json_object_object_add(*writer, "NTP_Server", json_object_new_string((pServer == NULL) ? "" : pServer));
  json_object_object_add(*writer, "Time", json_object_new_int64(Tools::GetTime()));
  *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
  return true;
}
bool NTP::SetTimeSettingsCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  *writer = json_object_new_object();
  FILE *File = fopen("/usr/share/zoneinfo/zone.tab", "r");
  if(File != NULL) {
    json_object *Zones = json_object_new_array();
    char  *Line = NULL;
    size_t Len  = 0;
    while(getline(&Line, &Len, File) != -1) {
      if(Line[0] == '#') continue;
      char  *Tmp  = Line;
      strtok_r(Tmp, "\t", &Tmp);
      strtok_r(Tmp, "\t", &Tmp);
      char *Result = strtok_r(Tmp, "\t", &Tmp);
      if(Result) json_object_array_add(Zones, json_object_new_string(Result));
    }
    if(Line != NULL) free(Line);
    fclose(File);
    json_object_object_add(*writer, "TimeZones", Zones);
  }
  File = fopen("/etc/timezone", "r");
  if(File != NULL) {
    char  *Line = NULL;
    size_t Len  = 0;
    if(getline(&Line, &Len, File) != -1) {
      if((strlen(Line) > 0) && (Line[strlen(Line) - 1] == '\n')) Line[strlen(Line) - 1] = 0;
      json_object_object_add(*writer, "TimeZone", json_object_new_string(Line));
    }
    if(Line != NULL) free(Line);
    fclose(File);
  }
  json_object_object_add(*writer, "NTP_Enabled", json_object_new_boolean(pEnabled));
  json_object_object_add(*writer, "NTP_Server", json_object_new_string((pServer == NULL) ? "" : pServer));
  *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
  return true;
}