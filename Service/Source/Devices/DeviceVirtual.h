/* ================================== zMeter ===========================================================================
 * File:     CaptureVirtual.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "DeviceInterface.h"
#include "Analyzers/AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
#define CAPTUREVIRTUAL_SAMPLEFREQ      6000.00        //Frequencia de muestreo
#define CAPTUREVIRTUAL_BUFFERSIZE     60000           //Tamaño del buffer (100 segundos)
#define CAPTUREVIRTUAL_SAMPLESREAD       64           //Numero de muestras generado cada vez
#define PROCESSING_FREQCYCLES           500           //Numero de ciclos usados para calcular frequencia
#define PROCESSING_CURRENTOFFSET          0.10        //Corriente minima para realizar calculos
//----------------------------------------------------------------------------------------------------------------------
class DeviceVirtual : public DeviceInterface {
  private :
    static bool    pClassRegistered;
  private :
    AnalyzerSoft  *pAnalyzer;
    float          pSampleRate;
  private : 
    static void            *Capture_Thread(void *args);
    static DeviceInterface *CreateInstance(json_object *config);
  private :
    AnalyzerInterface *FindAnalyzer(const char *serial);
  public :
    ~DeviceVirtual();
};
//----------------------------------------------------------------------------------------------------------------------