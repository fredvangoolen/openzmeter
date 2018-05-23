/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include "DeviceInterface.h"
//----------------------------------------------------------------------------------------------------------------------
float DeviceInterface::ApplyFilter(float in, Filter_t* filter) {
  float out = in * filter->A0 + filter->Z1;
  filter->Z1 = in * filter->A1 + filter->Z2 - filter->B1 * out;
  filter->Z2 = in * filter->A2 - filter->B2 * out;
  return out;
}
DeviceInterface::DeviceInterface() {
  pAnalyzers = NULL;
  pAnalyzersCount = 0;
  pSamplerate = 20000;
}
DeviceInterface::~DeviceInterface() {
  for(int i = 0; i < pAnalyzersCount; i++) delete pAnalyzers[i];
  free(pAnalyzers);
}