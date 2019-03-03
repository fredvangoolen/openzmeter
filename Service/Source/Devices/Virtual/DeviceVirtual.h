/* ================================== zMeter ===========================================================================
 * File:     CaptureVirtual.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "DeviceInterface.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
#define CAPTUREVIRTUAL_SAMPLEFREQ      6000.00        //Frequencia de muestreo
#define CAPTUREVIRTUAL_BUFFERSIZE     60000           //Tamaño del buffer (10 segundos)
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class DeviceVirtual : public DeviceInterface {
  private :
    static bool    pClassRegistered;
  private :
    AnalyzerSoft                   *pAnalyzer;
    AnalyzerInterface::AnalyzerMode pMode;
    float                           pSampleRate;
    float                           pVoltage;
    float                           pFrequency;
    pthread_mutex_t                 pMutex;
    json_object                    *pConfig;
  private : 
    static bool  Init();
    static void *Capture_Thread(void *args);
  private :
    bool               ConfigDev(json_object *config);
    AnalyzerInterface *FindAnalyzer(string serial);
    uint32_t           AnalyzersCount();
    string             Serial();
    string             Driver();
    string             Name();
    bool               Start();
    bool               Stop();

  public :
    ~DeviceVirtual();
};
//----------------------------------------------------------------------------------------------------------------------
