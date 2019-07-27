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
    static bool GetInstances();
    static void GetRemoteCallback(HTTPRequest &req);
    static void SetRemoteCallback(HTTPRequest &req);
//    static bool SetRemoteAnalyzerCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
//    static bool SetRemoteDataCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
  private :
    bool StartDevice();
    bool StopDevice();
  public :
    DeviceRemote();
    ~DeviceRemote();
};
//----------------------------------------------------------------------------------------------------------------------
