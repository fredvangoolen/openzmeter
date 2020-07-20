// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#include "AnalyzerSoftAggreg200MS.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAggreg200MS::AnalyzerSoftAggreg200MS(const string &serial, const Analyzer::AnalyzerMode mode) : AnalyzerSoftAggreg(serial, mode) {
  Reset();
}
AnalyzerSoftAggreg200MS::~AnalyzerSoftAggreg200MS() {
}
void AnalyzerSoftAggreg200MS::Save() const{
  Database DB(pSerial);
  DB.ExecSentenceNoResult("INSERT INTO aggreg_200ms(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, power_factor, thd_v, thd_i, phi, unbalance) VALUES(" +
  DB.Bind(pTimestamp) + ", " +
  DB.Bind(pVoltage, pVoltageChannels) + ", " +
  DB.Bind(pCurrent, pCurrentChannels) + ", " +
  DB.Bind(pFrequency) + ", " +
  DB.Bind(pFlag) + ", " +
  DB.Bind(pActive_Power, pVoltageChannels) + ", " +
  DB.Bind(pReactive_Power, pVoltageChannels) + ", " +
  DB.Bind(pPower_Factor, pVoltageChannels) + ", " +
  DB.Bind(pVoltage_THD, pVoltageChannels) + ", " +
  DB.Bind(pCurrent_THD, pCurrentChannels) + ", " +
  DB.Bind(pPhi, pVoltageChannels) + ", " +
  DB.Bind(pUnbalance) + ")");
}
void AnalyzerSoftAggreg200MS::Reset() {
  AnalyzerSoftAggreg::Reset();
}
void AnalyzerSoftAggreg200MS::setTimestamp(const uint64_t time) {
  pTimestamp = time;
}
void AnalyzerSoftAggreg200MS::setFrequency(const float val) {
  pFrequency = val;
}
void AnalyzerSoftAggreg200MS::setFlag(const bool val) {
  pFlag |= val;
}
void AnalyzerSoftAggreg200MS::setUnbalance(const float val) {
  pUnbalance = val;
}
void  AnalyzerSoftAggreg200MS::setVoltage(const uint8_t channel, const float val) {
  pVoltage[channel] = val;
}
void AnalyzerSoftAggreg200MS::setCurrent(const uint8_t channel, const float val) {
  pCurrent[channel] = val;
}
void AnalyzerSoftAggreg200MS::setActivePower(const uint8_t channel, const float val) {
  pActive_Power[channel] = val;
}
void AnalyzerSoftAggreg200MS::setApparentPower(const uint8_t channel, const float val) {
  pApparent_Power[channel] = val;
}
void AnalyzerSoftAggreg200MS::setReactivePower(const uint8_t channel, const float val) {
  pReactive_Power[channel] = val;
}
void AnalyzerSoftAggreg200MS::setActiveEnergy(const uint8_t channel, const float val) {
  pActive_Energy[channel] = val;
}
void AnalyzerSoftAggreg200MS::setReactiveEnergy(const uint8_t channel, const float val) {
  pReactive_Energy[channel] = val;
}
void AnalyzerSoftAggreg200MS::setVoltageHarmonic(const uint8_t channel, const uint8_t harmonic, const float val) {
  pVoltage_Harmonics[channel * 50 + harmonic] = val;
}
void AnalyzerSoftAggreg200MS::setCurrentHarmonic(const uint8_t channel, const uint8_t harmonic, const float val) {
  pCurrent_Harmonics[channel * 50 + harmonic] = val;
}
void AnalyzerSoftAggreg200MS::setPowerHarmonic(const uint8_t channel, const uint8_t harmonic, const float val) {
  pPower_Harmonics[channel * 50 + harmonic] = val;
}
void AnalyzerSoftAggreg200MS::setCurrentTHD(const uint8_t channel, const float val) {
  pCurrent_THD[channel] = val;
}
void AnalyzerSoftAggreg200MS::setVoltageTHD(const uint8_t channel, const float val) {
  pVoltage_THD[channel] = val;
}
void  AnalyzerSoftAggreg200MS::setVoltagePhase(const uint8_t channel, const float val) {
  pVoltage_Phase[channel] = val;
}
void  AnalyzerSoftAggreg200MS::setPhi(const uint8_t channel, const float val) {
  pPhi[channel] = val;
}
void  AnalyzerSoftAggreg200MS::setPowerFactor(const uint8_t channel, const float val) {
  pPower_Factor[channel] = val;
}
bool AnalyzerSoftAggreg200MS::AppendToJSON(const string &serie, json &result) const{
//  Result["Sampletime"] = pAggreg200MS.Timestamp;
  if(serie == "FREQUENCY") {
    result["Frequency"] = pFrequency;
  } else if(serie == "FLAG") {
    result["Flag"] = pFlag;
  } else if(serie == "UNBALANCE") {
    result["Unbalance"] = pUnbalance;
  } else if(serie == "VOLTAGE") {
    json Tmp = json::array();
    result["Voltage"] = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pVoltage[n]);
    result["Voltage"] = Tmp;
  } else if(serie == "CURRENT") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pCurrentChannels; n++) Tmp.push_back(pCurrent[n]);
    result["Current"] = Tmp;
  } else if(serie == "ACTIVE_POWER") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pActive_Power[n]);
    result["Active_Power"] = Tmp;
  } else if(serie == "REACTIVE_POWER") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pReactive_Power[n]);
    result["Reactive_Power"] = Tmp;
  } else if(serie == "ACTIVE_ENERGY") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pActive_Energy[n]);
    result["Active_Energy"] = Tmp;
  } else if(serie == "REACTIVE_ENERGY") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pReactive_Energy[n]);
    result["Rective_Energy"] = Tmp;
  } else if(serie == "APPARENT_POWER") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pApparent_Power[n]);
    result["Apparent_Power"] = Tmp;
  } else if(serie == "POWER_FACTOR") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pPower_Factor[n]);

    result["Power_Factor"] = Tmp;
  } else if(serie == "VOLTAGE_THD") {
    result["Voltage_THD"] = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) result["Voltage_THD"].push_back(pVoltage_THD[n]);
  } else if(serie == "CURRENT_THD") {
    result["Current_THD"] = json::array();
    for(uint8_t n = 0; n < pCurrentChannels; n++) result["Current_THD"].push_back(pCurrent_THD[n]);
  } else if(serie == "PHI") {
    result["Phi"] = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) result["Phi"].push_back(pPhi[n]);
  } else if(serie == "VOLTAGE_PHASE") {
    result["Voltage_Phase"] = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) result["Voltage_Phase"].push_back(pVoltage_Phase[n]);
//  } else if(serie == "VOLTAGE_HARMONICS") {
//    result["Voltage_Harmonics"] = json::array();
//    for(uint8_t n = 0; n < pVoltageChannels; n++) {
//      result["Voltage_Harmonics"].push_back(json::array());
//      for(uint8_t i = 0; i < 50; i++) {
//        result["Voltage_Harmonics"][n].push_back(json::array());
//        AnalyzerSoftFFT::Complex_t Complex = pVoltageFFT[n]->Harmonic(i);
//        Result["Voltage_Harmonics"][n][i].push_back(Complex.Re);
//        Result["Voltage_Harmonics"][n][i].push_back(Complex.Im);
//      }
//    }
//  } else if(Serie == "CURRENT_HARMONICS") {
//      Result["Current_Harmonics"] = json::array();
//      for(uint8_t n = 0; n < pCurrentChannels; n++) {
//        Result["Current_Harmonics"].push_back(json::array());
//        for(uint8_t i = 0; i < 50; i++) {
//          Result["Current_Harmonics"][n].push_back(json::array());
//          AnalyzerSoftFFT::Complex_t Complex = pCurrentFFT[n]->Harmonic(i);
//          Result["Current_Harmonics"][n][i].push_back(Complex.Re);
//          Result["Current_Harmonics"][n][i].push_back(Complex.Im);
//        }
//      }
//    } else if(Serie == "POWER_HARMONICS") {
//      Result["Power_Harmonics"] = json::array();
//      for(uint8_t n = 0; n < pVoltageChannels; n++) {
//        Result["Power_Harmonics"].push_back(json::array());
//        for(uint8_t i = 0; i < 50; i++) Result["Power_Harmonics"][n].push_back(pAggreg200MS.Power_Harmonics[n * 50 + i]);
//      }
//    } else if(Serie == "VOLTAGE_FFT") {
//      Result["Voltage_FFT"] = json::array();
//      for(uint8_t n = 0; n < pVoltageChannels; n++) {
//        Result["Voltage_FFT"].push_back(json::array());
//        uint16_t BlockSize = pVoltageFFT[n]->OutLen();
//        for(uint16_t i = 0; i < BlockSize; i++) Result["Voltage_FFT"][n].push_back(pVoltageFFT[n]->BinPower(i));
//      }
//    } else if(Serie == "CURRENT_FFT") {
//      Result["Current_FFT"] = json::array();
//      for(uint8_t n = 0; n < pCurrentChannels; n++) {
//        Result["Current_FFT"].push_back(json::array());
//        uint16_t BlockSize = pCurrentFFT[n]->OutLen();
//        for(uint16_t i = 0; i < BlockSize; i++) Result["Current_FFT"][n].push_back(pCurrentFFT[n]->BinPower(i));
//      }
//    } else if(Serie == "VOLTAGE_SAMPLES") {
//      Result["Voltage_Samples"] = json::array();
//      for(uint8_t n = 0; n < pVoltageChannels; n++) {
//        Result["Voltage_Samples"].push_back(json::array());
//        uint16_t BlockSize = pInputBlock->BlockSamples();
//        for(uint16_t i = 0; i < BlockSize; i++) Result["Voltage_Samples"][n].push_back(pInputBlock->VoltageSample(n, i));
//      }
//    } else if(Serie == "CURRENT_SAMPLES") {
//      Result["Current_Samples"] = json::array();
//      for(uint8_t n = 0; n < pCurrentChannels; n++) {
//        Result["Current_Samples"].push_back(json::array());
//        uint16_t BlockSize = pInputBlock->BlockSamples();
//        for(uint16_t i = 0; i < BlockSize; i++) Result["Current_Samples"][n].push_back(pInputBlock->CurrentSample(n, i));
//      }
//    } else if(Serie == "GA_POWER") {
//      Result["GA_Power"] = CalcGA();
//    } else if(Serie == "SYS_STATS") {
//      Result["Sys_Stats"] = SysSupport::GetSystemStatusJSON();
//    } else if(Serie == "UNREAD") {
//      string UserName = params["UserName"];
//      Result["Unread"] = json::object();
//      Database DB(pSerial);
//      DB.ExecSentenceAsyncResult("SELECT eventsread, (SELECT count(*) FROM events WHERE starttime > eventsread) AS events, alarmsread, (SELECT count(*) FROM alarms WHERE alarmtime > alarmsread) AS alarms FROM permissions WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
//      if(DB.FecthRow() == false) {
//        Result["Unread"]["EventsFrom"] = 0;
//        Result["Unread"]["Events"] = 0;
//        Result["Unread"]["AlarmsFrom"] = 0;
//        Result["Unread"]["Alarms"] = 0;
//      } else {
//        Result["Unread"]["EventsFrom"] = DB.ResultUInt64("eventsread");
//        Result["Unread"]["Events"] = DB.ResultUInt64("events");
//        Result["Unread"]["AlarmsFrom"] = DB.ResultUInt64("alarmsread");
//        Result["Unread"]["Alarms"] = DB.ResultUInt64("alarms");
//      }
  } else {
    return false;
  }
  return true;
}
// ---------------------------------------------------------------------------------------------------------------------