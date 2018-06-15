/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <pthread.h>
#include <inttypes.h>
#include <fftw3.h>
#include <json-c/json.h>
#include "Database.h"
#include "FFT.h"
//----------------------------------------------------------------------------------------------------------------------
#define ZEROCROSSING_FILTERSTAGES          3
#define ZEROCROSSING_SECONDS              10        //Numero de segundos usados para calcular frequencia
//----------------------------------------------------------------------------------------------------------------------
class ZeroCrossing {
  private :
    typedef struct {
      float Z1;
      float Z2;
    } FilterStage_t;
    typedef struct {
      float         A0;
      float         A1;
      float         A2;
      float         B1;
      float         B2;
      FilterStage_t Channels[3][ZEROCROSSING_FILTERSTAGES];
    } Filter_t;
  private :
    float    pLastSamples[3];
    float    pSamplerate;
    bool     p3Phase;
    Filter_t pFilters;
  public :
    ZeroCrossing(bool phase3, float samplerate);
    bool  ProcessSample(float *samples);
    float Frequency();
};