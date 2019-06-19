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
  public :
    static json GetSystemDetails();     
  public:
    System();
    void PeriodicTask();
    virtual ~System();
};