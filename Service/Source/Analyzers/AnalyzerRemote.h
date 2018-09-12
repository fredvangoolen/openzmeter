/* ================================== zMeter ===========================================================================
 * File:     AnalyzerRemote.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "AnalyzerInterface.h"
class AnalyzerRemote : public AnalyzerInterface {
  protected :
    void GetParams(Params_t *params, ExtParams_t *extParams);
  public :
    AnalyzerRemote(char *id, float nominalVoltage, float nominalFreq, char *name, float maxCurrent, float minCurrent, AnalyzerMode mode, float sampleFreq, bool online);
    void UpdateConfig(float nominalVoltage, float nominalFreq, char *name, float maxCurrent, float minCurrent, AnalyzerMode mode, float sampleFreq, bool online);
    bool SaveJSON(json_object *json, Database *db);
};