/* ================================== zMeter ===========================================================================
 * File:     PricesInterface.cpp
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "Tools.h"
#include "PricesInterface.h"
//----------------------------------------------------------------------------------------------------------------------
vector<PricesInterface::Type_t> PricesInterface::pRegisteredTypes;
//----------------------------------------------------------------------------------------------------------------------
PricesInterface::PricesInterface(const char *id) : Database(id) {
  pThread    = 0;
  pTerminate = false;
  pLastPrice = 0;
  ExecSentence("SELECT MAX(sampletime) AS sampletime FROM (SELECT MAX(sampletime) AS sampletime FROM prices UNION SELECT MIN(sampletime) - 3600000 FROM aggreg_1h) AS Tmp");
  const char *Value = ResultValue(0, "sampletime");
  if(Value != NULL) pLastPrice = strtoull(Value, NULL, 10);
}
void PricesInterface::StartThread() {
  if(pthread_create(&pThread, NULL, Update_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  }
  pthread_setname_np(pThread, "PriceUpdate");  
}
bool PricesInterface::RegisterType(const char* name, ConstructorFunction constructor) {
  Type_t Type = { name, constructor };
  pRegisteredTypes.push_back(Type);
  Tools::LogMsg("INFO", "Registering '%s' price interface plugin\n", name);
  return true;
}
PricesInterface *PricesInterface::Config(const char *name, json_object *config) {
  if(config == NULL) return NULL;
  char *Name = NULL;
  if(Tools::JSONString(config, "Driver", &Name) == false) {
    Tools::LogMsg("ERROR", "Prices 'Driver' parameter not specified\n");
  } else {
    for(vector<Type_t>::iterator it = pRegisteredTypes.begin() ; it != pRegisteredTypes.end(); ++it) {
      if(strcmp(Name, it->Name) == 0) return it->Constructor(name, config);
    }
  }
  return NULL;
}
void PricesInterface::SavePrice(uint64_t hour, float price) {
  pLastPrice = (hour / 3600000) * 3600000 + 3599999;
  PrepareSentence("INSERT INTO prices(sampletime, price) VALUES(?, ?)");
  BindSingleUInt64(pLastPrice);
  BindSingleFloat(price);
  ExecSentence();  
}
void *PricesInterface::Update_Thread(void* args) {
  uint64_t LastQuery = 0;
  PricesInterface *This = (PricesInterface*)args;
  while(!This->pTerminate) {
    uint64_t Now = Tools::GetTime();
    if(This->pLastPrice < Now) {
      Tools::LogMsg("DEBUG", "Check prices for %"PRIu64"\n", This->pLastPrice + 1);
      This->CheckPrices(This->pLastPrice + 1);
      LastQuery = Tools::GetTime();
    } else if((This->pLastPrice > Now) && ((Now - LastQuery) > 600000)) {
      This->CheckPrices(This->pLastPrice + 1);
      LastQuery = Tools::GetTime();
    }    
    usleep(1000000);
  }
  return NULL;
}