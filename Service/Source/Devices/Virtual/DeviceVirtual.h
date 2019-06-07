/* ================================== zMeter ===========================================================================
 * File:     CaptureVirtual.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Devices.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class DeviceVirtual : public Device {
  private :
    pthread_t                       pCaptureThread;
    bool                            pTerminate;
    AnalyzerSoft                   *pAnalyzer;
    AnalyzerInterface::AnalyzerMode pMode;
    float                           pSampleRate;
    float                           pVoltage;
    float                           pFrequency;
    pthread_mutex_t                 pMutex;
  private : 
    static void *Capture_Thread(void *args);
  private :
//    bool               SetConfig(json config);
    AnalyzerInterface *FindAnalyzer(string serial);
    uint32_t           AnalyzersCount();
    bool               Start();
    bool               Stop();
  public :
    static void GetInstances(vector<Device*> &instances);
  public :
    DeviceVirtual();
    ~DeviceVirtual();
};
//----------------------------------------------------------------------------------------------------------------------
