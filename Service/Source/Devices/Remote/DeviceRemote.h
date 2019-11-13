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
class DeviceRemote final : public Device {
  private :
    static bool          pRegistered;
    static DeviceRemote *pInstance;
  private :
    static bool GetInstances();
    static void GetRemoteCallback(HTTPRequest &req);
    static void SetRemoteCallback(HTTPRequest &req);
    static json CheckConfig(const json &config);
  private :
    DeviceRemote();
//    bool Start(const json &config);
    bool StartDevice();
    bool StopDevice();
    void GetDevice(HTTPRequest &req);
    void SetDevice(HTTPRequest &req);
    void GetRemote(HTTPRequest &req);
    void SetRemote(HTTPRequest &req);
  public :
    ~DeviceRemote();
};
//----------------------------------------------------------------------------------------------------------------------
