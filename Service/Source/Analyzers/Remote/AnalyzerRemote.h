/* ================================== zMeter ===========================================================================
 * File:     AnalyzerRemote.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Analyzers.h"
class AnalyzerRemote : public AnalyzerInterface {
  protected :
//    void GetParams(Params_t &params, bool frequency, bool flag, bool rms_v, bool rms_i, bool active_power, bool reactive_power, bool active_energy,
//                                     bool reactive_energy, bool apparent_power, bool power_factor, bool thd_v, bool thd_i, bool phi, bool voltage_harmonics, 
//                                     bool current_harmonics, bool fft_v, bool fft_i, bool voltage, bool current);
  public :
    AnalyzerRemote(string id, AnalyzerInterface::AnalyzerMode mode, float samplerate);
//    void UpdateConfig(float nominalVoltage, float nominalFreq, char *name, float maxCurrent, float minCurrent, AnalyzerMode mode, float sampleFreq, bool online);
//    bool SaveJSON(json_object *json, Database *db);
};