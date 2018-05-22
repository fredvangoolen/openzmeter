/* ================================== zMeter ===========================================================================
 * File:     Prices.cpp
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
#include "Prices.h"
/* Genera consultas de forma periodica a la pagina de REE para descargar las tarifas y calcular el precio del consumo */
Prices::Prices(uint32_t interval) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  pTerminate = false;
  pLastCheck = 0;
  
  /*SELECT MAX(sampletime) FROM (
  SELECT MAX(sampletime / 1000) AS sampletime FROM prices
UNION
  SELECT MIN(sampletime / 1000) FROM aggreg_1h) AS Tmp;*/
  
  if(pthread_create(&pThread, NULL, Update_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    curl_global_cleanup();
    return;
  }
}
Prices::~Prices() {
  pTerminate = true;
  pthread_join(pThread, NULL);
  curl_global_cleanup();
}
void *Prices::Update_Thread(void* args) {
  Prices *This = (Prices*)args;
  pthread_setname_np(This->pThread, "PriceServer");
  while(!This->pTerminate) {
//    uint32_t CurrentTimestamp = (uint32_t)time(NULL);
    usleep(1000000);
//
//
//    Timestamp = (Timestamp / 86400) * 86400;
//    if(Timestamp > CurrentTimestamp) continue;
//    CURL *Curl = curl_easy_init();
//    struct tm Time;
//    gmtime_r((time_t*)&Timestamp, &Time);
//    MemBuffer_t Buffer;
//    Buffer.Buffer = NULL;
//    Buffer.Size = 0;
//    char URL[1024];
//    int Len = sprintf(URL, "https://api.esios.ree.es/archives/80/download?date_type=datos&locale=es");
//    Len    += strftime(&URL[Len], 100, "&start_date=%Y-%m-%dT00:00:00+00:00", &Time);
//    Len    += strftime(&URL[Len], 100, "&end_date=%Y-%m-%dT23:59:59+00:00", &Time);
//    curl_easy_setopt(Curl, CURLOPT_URL, URL);
//    curl_easy_setopt(Curl, CURLOPT_SSL_VERIFYPEER, 0L);
//    curl_easy_setopt(Curl, CURLOPT_SSL_VERIFYHOST, 0L);
//    curl_easy_setopt(Curl, CURLOPT_WRITEDATA, &Buffer);
//    curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, Prices_ReadCallback);
//    CURLcode Result = curl_easy_perform(Curl);
//    if(Result != CURLE_OK) {
////      LogMsg("ERROR", "curl_easy_perform() failed: %s\n", curl_easy_strerror(Result));
//      curl_easy_cleanup(Curl);
//      continue;
//    }
//    xmlDoc *Doc = xmlReadMemory(Buffer.Buffer, Buffer.Size, URL, NULL, 0);
//    //Lo que quiero es la serie "FEU" con tarifa "Z01", "Z02" o "Z03" segun seleccione
//    if(Doc != NULL) {
//      xmlXPathContext *Context = xmlXPathNewContext(Doc);
//      xmlXPathRegisterNs(Context, "ns", "http://sujetos.esios.ree.es/schemas/2014/04/01/PVPCDesgloseHorario-esios-MP/");
//      if(Context != NULL) {
//        xmlChar *xPath = (xmlChar*)"/ns:PVPCDesgloseHorario/ns:SeriesTemporales[ns:TerminoCosteHorario[@v='FEU'] and ns:TipoPrecio[@v='Z01']]/ns:Periodo/ns:Intervalo/ns:Ctd";
//        xmlXPathObject *Result = xmlXPathEvalExpression(xPath, Context);
//        xmlXPathFreeContext(Context);
//	      if(Result != NULL) {
//          xmlNodeSet *NodeSet = Result->nodesetval;
//          float *Prices = malloc(sizeof(float) * 24);
//          if(NodeSet->nodeNr == 24) {
//            for(int i = 0; i < NodeSet->nodeNr; i++) Prices[i] = atof(xmlGetProp(NodeSet->nodeTab[i], "v"));
//            char StrDate[64];
//            strftime(StrDate, sizeof(StrDate), "%d/%m/%Y", &Time);
////            LogMsg("INFO", "Actualizando precios del dia %s\n", StrDate);
////            DB_Save(PRICES, 0, Timestamp, Prices, true);
//            Timestamp += 86400;
//          }
//          xmlXPathFreeObject(Result);
//        }
//      }
//      fflush(stdout);
//      xmlFreeDoc(Doc);
//      xmlCleanupParser();
    }
//    free(Buffer.Buffer);
//    curl_easy_cleanup(Curl);
  return NULL;
}





// Tipos privados ======================================================================================================
typedef struct {
  char  *Buffer;
  size_t Size;
} MemBuffer_t;


//======================================================================================================================
size_t Prices_ReadCallback(void *contents, size_t size, size_t nmemb, void *userp) {
//  size_t realsize = size * nmemb;
//  MemBuffer_t *mem = (MemBuffer_t*)userp;
//  mem->Buffer = realloc(mem->Buffer, mem->Size + realsize + 1);
//  if(mem->Buffer == NULL) {
////    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    return 0;
//  }
//  memcpy(&(mem->Buffer[mem->Size]), contents, realsize);
//  mem->Size += realsize;
//  mem->Buffer[mem->Size] = 0;
//  return realsize;
  return 0;
}
//======================================================================================================================

