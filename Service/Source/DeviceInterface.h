/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Analyzer.h"
//----------------------------------------------------------------------------------------------------------------------
class DeviceInterface {
  public :
    enum RouteType {FASE_1, FASE_3, FASE_3N};
    typedef struct {
      char             *Type;
      char             *Serial;
      char             *Name;
      float             VoltageGain;
      int               VoltageOffset;
      float             CurrentGain;
      int               CurrentOffset;
      Analyzer *Instance;
    } Analyzer_t;
    typedef struct {
      Analyzer_t      **Items;
      uint32_t          Len;
    } Analyzers_t;
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
    Analyzers_t        Analyzers;
    float              Samplerate;
  private :
    static float  ApplyFilter(float in, Filter_t *filter);
//    CaptureInterface();
//    virtual ~CaptureInterface();
//    virtual bool ReadConfig(json_object *config) = 0;
  public :
//    static bool Init(json_object *config);
    virtual bool Start() = 0;
};

//----------------------------------------------------------------------------------------------------------------------