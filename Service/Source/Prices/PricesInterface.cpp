/* ================================== zMeter ===========================================================================
 * File:     PricesInterface.cpp
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "PricesInterface.h"
PricesInterface::PricesInterface(AnalyzerInterface* analyzer, const char* units) : Database(analyzer->GetID()) {
  pTerminate = false;
  pLastPrice = 0;

}