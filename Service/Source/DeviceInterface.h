/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Analyzer.h"
//----------------------------------------------------------------------------------------------------------------------
class DeviceInterface {
  public :
    typedef struct {
      float A0;
      float A1;
      float A2;
      float B1;
      float B2;
      float Z1;
      float Z2;
    } Filter_t;
  public :
    Analyzer       **pAnalyzers;             //Analizadores disponibles
    uint32_t         pAnalyzersCount;        //Numero de analizadores
    float            pSamplerate;            //Velocidad de muestreo
    uint32_t         pID;                    //Identificador del dispositivo (Tiene que ser multiplo de 4)
  private :
    static float  ApplyFilter(float in, Filter_t *filter);
  protected :
    DeviceInterface();
    virtual ~DeviceInterface();
  public :
    virtual bool Start() = 0;
};

//----------------------------------------------------------------------------------------------------------------------