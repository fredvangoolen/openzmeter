/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
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
using namespace std;
class ZeroCrossing {
  private :

  private :
//    Filter_t               pFilters;
    float                  pSamplerate;
    float                  pNominalFrequency;
//    uint32_t               pMaxSamples;
//    uint32_t               pCyclesPos;
//    uint32_t               pBlockLen;
//    uint32_t               pCycleLen;
  public :
//    ZeroCrossing(uint8_t channels, float samplerate, float freq);
//    ~ZeroCrossing();
//    bool  ProcessSample(const vector<float> &samples);
//    float Frequency();
};