#pragma once
#include <string>
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class System {
  private :
    static System          *pInstance;
    static pthread_mutex_t  pMutex;
    static bool             pRegistered;
  private :
    float            pBattery;
    bool             pCharging;
    bool             pPowered;
  public :
    static json GetSystemDetails();
    static void SetBatteryLevel(float level, bool charging, bool powered);
  public:
    System();
    void PeriodicTask();
    virtual ~System();
};