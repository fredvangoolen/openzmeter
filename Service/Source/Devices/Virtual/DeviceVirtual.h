/* ================================== zMeter ===========================================================================
 * File:     CaptureVirtual.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Devices.h"
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
    AnalyzerSoft                   *pAnalyzer;
    AnalyzerInterface::AnalyzerMode pMode;
    float                           pSampleRate;
    float                           pVoltage;
    float                           pFrequency;
    pthread_mutex_t                 pMutex;
  private : 
    static bool GetInstances();
    static void *Capture_Thread(void *args);
  private :
    AnalyzerInterface *FindAnalyzer(string serial);
    uint32_t           AnalyzersCount();
    bool               Start();
    bool               Stop();
  public :
    DeviceVirtual();
    ~DeviceVirtual();
};
//----------------------------------------------------------------------------------------------------------------------
