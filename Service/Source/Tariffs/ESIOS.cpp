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
#include "ESIOS.h"
// ---------------------------------------------------------------------------------------------------------------------
string                    ESIOS::pToken       = "";
pthread_t                 ESIOS::pThread      = 0;
pthread_mutex_t           ESIOS::pMutex       = PTHREAD_MUTEX_INITIALIZER;
bool                      ESIOS::pTerminate   = false;
// ---------------------------------------------------------------------------------------------------------------------
ESIOS::ESIOS() {
  HTTPServer::RegisterCallback("getESIOS", GetESIOSCallback);
  HTTPServer::RegisterCallback("setESIOS", SetESIOSCallback);
  Configure(CheckConfig(ConfigManager::ReadConfig("/ESIOS")));
//  curl_global_init(CURL_GLOBAL_DEFAULT);
//  pCurlBuff.Buffer  = NULL;
//  pCurlBuff.MaxSize = 0;
//  pCurlBuff.Size    = 0;
//  pCurl = curl_easy_init();
//  curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)&pCurlBuff);
//  curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, Curl_Read_Callback);
//  pRate = "Z01";
}
ESIOS::~ESIOS() {
  pthread_mutex_lock(&pMutex);
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  pthread_mutex_unlock(&pMutex);
}
json ESIOS::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Token"] = "";
  ConfigOUT["Enabled"] = false;
  if(config.is_object() == true) {
    if(config.contains("Token") && config["Token"].is_string()) ConfigOUT["Token"] = config["Token"];
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
  }
  return ConfigOUT;
}
void ESIOS::Configure(const json &config) {
  ConfigManager::WriteConfig("/ESIOS", config);
  bool Enable = config["Enabled"];
  pthread_mutex_lock(&pMutex);
  pToken = config["Token"];
  if((Enable == true) && (pThread == 0)) {
    pTerminate = false;
    if(pthread_create(&pThread, NULL, Thread_Func, NULL) != 0) {
      pThread = 0;
      LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    } else {
      pthread_setname_np(pThread, "ESIOSClient");
    }
  } else if((Enable == false) && (pThread != 0)) {
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
  }
  pthread_mutex_unlock(&pMutex);
}
void ESIOS::GetESIOSCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigManager::ReadConfig("/ESIOS");
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void ESIOS::SetESIOSCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Enabled") == false) || (params["Enabled"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Enabled' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Token") == false) || (params["Token"].is_string() == false)) return HTTPServer::FillResponse(response, "'Token' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Configure(CheckConfig(params));
  HTTPServer::FillResponse(response, "", "application/json", HTTP_OK);
}
void *ESIOS::Thread_Func(void *params) {
  while(pTerminate == false) {
    usleep(100000);
  }
  return NULL;
}

//PricesInterface *ESIOS::CreateInstance(const char *name, json_object *config) {
//  ESIOS *This = new ESIOS(name);
  //Leo si estoy en Z01...
//  Tools::LogMsg("INFO", "Initiated ESIOS price interface\n");
//  This->StartThread();
//  return This;
//}
//void ESIOS::XMLError(void *ctx, const char *msg,...) {}
//ESIOS::~ESIOS() {
//  curl_easy_cleanup(pCurl);
//  if(pCurlBuff.Buffer) free(pCurlBuff.Buffer);
//  curl_global_cleanup();
//}
//size_t ESIOS::Curl_Read_Callback(void *contents, size_t size, size_t nmemb, void *userp) {
//  CurlBuff_t *Buff = (CurlBuff_t*)userp;
//  if((Buff->Size + size * nmemb + 1) > Buff->MaxSize) {
//    Buff->Buffer = (char*)realloc(Buff->Buffer, Buff->Size + size * nmemb + 1);
//    if(Buff->Buffer == NULL) {
//      return 0;
//    } else {
//      Buff->MaxSize = Buff->Size + size * nmemb + 1;
//    }
//  }
//  memcpy(&Buff->Buffer[Buff->Size], contents, size * nmemb);
//  Buff->Size += size * nmemb;
//  Buff->Buffer[Buff->Size] = 0;
//  return size * nmemb;
//}
//void ESIOS::CheckPrices(uint64_t from) {
//  long   HTTPCode;
//  char   Tmp[256];
//  time_t Timestamp = ((from / 1000) / 86400) * 86400;
//  struct tm Time;
//  pCurlBuff.Size = 0;
//  gmtime_r(&Timestamp, &Time);
//  strftime(Tmp, sizeof(Tmp), "https://api.esios.ree.es/archives/80/download?date=%Y-%m-%d", &Time);
//  xmlGenericErrorFunc ErrorHandler = XMLError;
//  initGenericErrorDefaultFunc(&ErrorHandler);
//  curl_easy_setopt(pCurl, CURLOPT_URL, Tmp);
//  CURLcode Result = curl_easy_perform(pCurl);
//  curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &HTTPCode);
//  if(Result != CURLE_OK) {
//    LOGFile::Error("curl_easy_perform() failed: %s\n", curl_easy_strerror(Result));
//    return;
//  }
//  if(HTTPCode != 200) {
//    LOGFile::Error("Query '%s' return code %d\n", Tmp, HTTPCode);
//    return;
//  }
//  xmlDoc *Doc = xmlReadMemory(pCurlBuff.Buffer, pCurlBuff.Size, Tmp, NULL, 0);
//  if(Doc != NULL) {
//    xmlXPathContext *Context = xmlXPathNewContext(Doc);
//    xmlXPathRegisterNs(Context, (const xmlChar*)"ns", (const xmlChar*)"http://sujetos.esios.ree.es/schemas/2014/04/01/PVPCDesgloseHorario-esios-MP/");
//    if(Context != NULL) {
//      snprintf(Tmp, sizeof(Tmp), "/ns:PVPCDesgloseHorario/ns:SeriesTemporales[ns:TerminoCosteHorario[@v='FEU'] and ns:TipoPrecio[@v='%s']]/ns:Periodo/ns:Intervalo/ns:Ctd", pRate);
//      xmlXPathObject *Result = xmlXPathEvalExpression((xmlChar*)Tmp, Context);
//      xmlXPathFreeContext(Context);
//      if(Result != NULL) {
//        xmlNodeSet *NodeSet = Result->nodesetval;
//        for(int i = 0; i < NodeSet->nodeNr; i++) {
//          float Price = atof((const char *)xmlGetProp(NodeSet->nodeTab[i], (const xmlChar*)"v"));
//          SavePrice((uint64_t)Timestamp * 1000 + (i * 3600000), Price);
//        }
//        Tools::LogMsg("INFO", "Price information downloaded from ESIOS for day %d/%d/%d\n", Time.tm_mday, Time.tm_mon + 1, Time.tm_year + 1900);
//        xmlXPathFreeObject(Result);
//      } else {
//        LOGFile::Error("No data found in response\n");
//      }
//    } else {
//      LOGFile::Error("Invalid XML context\n");
//    }
//    xmlFreeDoc(Doc);
//  } else {
//    LOGFile::Debug("No data for requested day (Invalid XML response)\n");
//  }
//}