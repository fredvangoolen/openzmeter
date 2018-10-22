/* ================================== zMeter ===========================================================================
 * File:     ESIOS.cpp
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include "Tools.h"
#include "ESIOS.h"
// Register ============================================================================================================
bool  ESIOS::pClassRegistered = PricesInterface::RegisterType("ESIOS", ESIOS::CreateInstance);
// Funciones ===========================================================================================================
ESIOS::ESIOS(const char* id) : PricesInterface(id) {
  Tools::LogMsg("DEBUG", "ESIOS price interface started\n");
  pRate = "Z01";
}
PricesInterface *ESIOS::CreateInstance(const char *name, json_object *config) {
  ESIOS *This = new ESIOS(name);
  //Leo si estoy en Z01...
  curl_global_init(CURL_GLOBAL_DEFAULT);
  This->pCurlBuff.Buffer  = NULL;
  This->pCurlBuff.MaxSize = 0;
  This->pCurlBuff.Size    = 0;
  This->pCurl = curl_easy_init();
  curl_easy_setopt(This->pCurl, CURLOPT_WRITEDATA, (void*)&This->pCurlBuff);
  curl_easy_setopt(This->pCurl, CURLOPT_WRITEFUNCTION, Curl_Read_Callback);
  This->StartThread();
  return This;
}
void ESIOS::XMLError(void *ctx, const char *msg,...) {}
ESIOS::~ESIOS() {
  curl_easy_cleanup(pCurl);
  if(pCurlBuff.Buffer) free(pCurlBuff.Buffer);
  curl_global_cleanup();
}
size_t ESIOS::Curl_Read_Callback(void *contents, size_t size, size_t nmemb, void *userp) {
  CurlBuff_t *Buff = (CurlBuff_t*)userp;
  if((Buff->Size + size * nmemb + 1) > Buff->MaxSize) {
    Buff->Buffer = (char*)realloc(Buff->Buffer, Buff->Size + size * nmemb + 1);
    if(Buff->Buffer == NULL) {
      return 0;
    } else {
      Buff->MaxSize = Buff->Size + size * nmemb + 1;
    }
  }
  memcpy(&Buff->Buffer[Buff->Size], contents, size * nmemb);
  Buff->Size += size * nmemb;
  Buff->Buffer[Buff->Size] = 0;
  return size * nmemb;
}
void ESIOS::CheckPrices(uint64_t from) {
  long   HTTPCode;
  char   Tmp[256];
  time_t Timestamp = ((from / 1000) / 86400) * 86400;
  struct tm Time;
  pCurlBuff.Size = 0;
  gmtime_r(&Timestamp, &Time);
  strftime(Tmp, sizeof(Tmp), "https://api.esios.ree.es/archives/80/download?date=%Y-%m-%d", &Time);
  xmlGenericErrorFunc ErrorHandler = XMLError;
  initGenericErrorDefaultFunc(&ErrorHandler);
  curl_easy_setopt(pCurl, CURLOPT_URL, Tmp);
  CURLcode Result = curl_easy_perform(pCurl);
  curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &HTTPCode);
  if((Result != CURLE_OK) || (HTTPCode != 200)) {
    Tools::LogMsg("ERROR", "curl_easy_perform() failed: %s\n", curl_easy_strerror(Result));
    return;
  } 
  xmlDoc *Doc = xmlReadMemory(pCurlBuff.Buffer, pCurlBuff.Size, Tmp, NULL, 0);
  if(Doc != NULL) {
    xmlXPathContext *Context = xmlXPathNewContext(Doc);
    xmlXPathRegisterNs(Context, (const xmlChar*)"ns", (const xmlChar*)"http://sujetos.esios.ree.es/schemas/2014/04/01/PVPCDesgloseHorario-esios-MP/");
    if(Context != NULL) {
      snprintf(Tmp, sizeof(Tmp), "/ns:PVPCDesgloseHorario/ns:SeriesTemporales[ns:TerminoCosteHorario[@v='FEU'] and ns:TipoPrecio[@v='%s']]/ns:Periodo/ns:Intervalo/ns:Ctd", pRate);
      xmlXPathObject *Result = xmlXPathEvalExpression((xmlChar*)Tmp, Context);
      xmlXPathFreeContext(Context);
      if(Result != NULL) {
        xmlNodeSet *NodeSet = Result->nodesetval;
        if(NodeSet->nodeNr == 24) {
          for(int i = 0; i < NodeSet->nodeNr; i++) {
            float Price = atof((const char *)xmlGetProp(NodeSet->nodeTab[i], (const xmlChar*)"v"));
            SavePrice((uint64_t)Timestamp * 1000 + (i * 3600000), Price);
          }
          Tools::LogMsg("INFO", "Price information downloaded from ESIOS for day %d/%d/%d\n", Time.tm_mday, Time.tm_mon + 1, Time.tm_year + 1900);
        }
        xmlXPathFreeObject(Result);
      }
    }
    xmlFreeDoc(Doc);
  }
}