/* ================================== zMeter ===========================================================================
 * File:     CaptureVirtual.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Device.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class DeviceVirtual final : public Device {
  private :
    static bool                     pRegistered;
  private :
    pthread_mutex_t                 pMutex;
    pthread_t                       pThread;
    bool                            pTerminate;
    Analyzer::AnalyzerMode          pMode;
    float                           pSampleRate;
    float                           pVoltage;
    float                           pCurrent;
    float                           pCurrentPhase;
    float                           pFrequency;
  private : 
    static bool  GetInstances();
    static void *Capture_Thread(void *args);
    static json  CheckConfig(const json &config);
  private :
    DeviceVirtual();
    bool Start(const json &config);
    bool StartDevice() override;
    bool StopDevice() override;
    void GetDevice(HTTPRequest &req) override;
    void SetDevice(HTTPRequest &req) override;
  public :
    ~DeviceVirtual() override;
};
//----------------------------------------------------------------------------------------------------------------------
