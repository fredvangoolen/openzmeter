/* ================================== zMeter ===========================================================================
 * File:     AnalyzerRemote.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Analyzers.h"
class AnalyzerRemote : public Analyzer {
  protected :
    void GetParams(Params_t& params, ParamsType& copy);
  public :
    AnalyzerRemote(string id, Analyzer::AnalyzerMode mode);
//    void UpdateConfig(float nominalVoltage, float nominalFreq, char *name, float maxCurrent, float minCurrent, AnalyzerMode mode, float sampleFreq, bool online);
//    bool SaveJSON(json_object *json, Database *db);
};