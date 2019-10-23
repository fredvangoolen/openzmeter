#include "Debian.h"
//----------------------------------------------------------------------------------------------------------------------
bool            System::pRegistered      = false;
System         *System::pInstance        = NULL;
pthread_mutex_t System::pMutex           = PTHREAD_MUTEX_INITIALIZER;
//----------------------------------------------------------------------------------------------------------------------
System::System() {
  pBattery         = NAN;
  pCharging        = false;
  pPowered         = false;
}
void System::PeriodicTask() {
}
json System::GetSystemDetails() {
  pthread_mutex_lock(&pMutex);
  json Result = json::object();
  if(pInstance != NULL) {
    Result["Battery"] = pInstance->pBattery;
    Result["Charging"] = pInstance->pCharging;
    Result["Powered"] = pInstance->pPowered;
  }
  pthread_mutex_unlock(&pMutex);
  return Result;
}
void System::SetBatteryLevel(float level, bool charging, bool powered) {
  pthread_mutex_lock(&pMutex);  
  if(pInstance != NULL) {
    pInstance->pBattery = level;
    pInstance->pCharging = charging;
    pInstance->pPowered = powered;
  }
  pthread_mutex_unlock(&pMutex);
}
System::~System() {
}

