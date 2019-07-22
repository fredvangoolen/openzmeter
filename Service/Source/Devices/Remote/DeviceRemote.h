/* ================================== zMeter ===========================================================================
 * File:     DeviceRemote.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <list>
#include "Device.h"
#include "AnalyzerRemote.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class DeviceRemote : public Device {
  private :
    static bool pRegistered;  
  private :
    list<AnalyzerRemote*>   pAnalyzers;               //Analyzers list
    pthread_mutex_t         pMutex;                   //Access to list limited
  private :
    static bool GetInstances();
//    static bool SetRemoteAnalyzerCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
//    static bool SetRemoteDataCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
  private :
    Analyzer *FindAnalyzer(string serial);
    uint32_t           AnalyzersCount();
    bool               Start();
    bool               Stop();
  public :
    DeviceRemote();
    ~DeviceRemote();
};
//----------------------------------------------------------------------------------------------------------------------
