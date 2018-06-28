/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.h
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Database.h"
class AnalyzerInterface : protected Database {
  protected :
    float             pNominalVoltage;
    float             pNominalFrequency;
  public :
    AnalyzerInterface(float nominalVoltage, float nominalFrequency, char *id);
};
