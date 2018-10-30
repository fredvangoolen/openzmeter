/* ================================== zMeter ===========================================================================
 * File:     DeviceRemote.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <libusb-1.0/libusb.h>
#include "DeviceInterface.h"
#include "AnalyzerRemote.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
class DeviceRemote : public DeviceInterface {
  private :
    typedef struct {
      char           *ID;
      char           *Name;
      AnalyzerRemote *Analyzer;
    } Analyzer_t;
    static Analyzer_t      *pAnalyzers;               //Analyzers list
    static uint32_t         pAnalyzersCount;          //Analyzers count
    static DeviceRemote    *pInstance;                //Only one instance allowed!
    static pthread_mutex_t  pListMutex;               //Access to list limited
  private :
    AnalyzerInterface *FindAnalyzer(char *serial);
  private :
    static bool SetRemoteAnalyzerCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool SetRemoteDataCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
  public :
    DeviceRemote(json_object* config);
    ~DeviceRemote();
};
//----------------------------------------------------------------------------------------------------------------------
