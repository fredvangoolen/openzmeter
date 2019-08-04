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
class DeviceVirtual : public Device {
  private :
    static bool pRegistered;
  private :
    pthread_t                       pCaptureThread;
    bool                            pTerminate;
    Analyzer::AnalyzerMode          pMode;
    float                           pSampleRate;
    float                           pVoltage;
    float                           pCurrent;
    float                           pCurrentPhase;
    float                           pFrequency;
    pthread_mutex_t                 pMutex;
  private : 
    static bool  GetInstances();
    static void *Capture_Thread(void *args);
  private :
    bool StartDevice();
    bool StopDevice();
    void GetDevice(HTTPRequest &req);
    void SetDevice(HTTPRequest &req);
  private :
    DeviceVirtual();
  public :
    ~DeviceVirtual();
};
//----------------------------------------------------------------------------------------------------------------------
