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
#include "Database.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAggreg200MS::AnalyzerSoftAggreg200MS(const string &serial, const Analyzer::AnalyzerMode mode) : AnalyzerSoftAggreg(serial, mode) {
  Reset();
}
AnalyzerSoftAggreg200MS::~AnalyzerSoftAggreg200MS() {
}
void AnalyzerSoftAggreg200MS::Save() const {
  Database DB(pSerial);
  DB.ExecSentenceNoResult("INSERT INTO aggreg_200ms(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, power_factor, thd_v, thd_i, phi, unbalance, harmonics_v, harmonics_i, harmonics_p) VALUES(" +
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
  DB.Bind(pUnbalance) + ", " +
  DB.Bind((float*)pVoltage_HarmonicsComplex, pVoltageChannels, 50, 2) + ", " +
  DB.Bind((float*)pCurrent_HarmonicsComplex, pCurrentChannels, 50, 2) + ", " +
  DB.Bind(pPower_Harmonics, pVoltageChannels, 50) + ")");
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
void AnalyzerSoftAggreg200MS::setVoltageHarmonic(const uint8_t channel, const uint8_t harmonic, const AnalyzerSoftAggreg200MS::Complex_t val) {
  pVoltage_Harmonics[channel * 50 + harmonic] = sqrt(val.Re * val.Re + val.Im * val.Im) / M_SQRT2 * 2.0;
  pVoltage_HarmonicsComplex[channel * 50 + harmonic] = val;
}
void AnalyzerSoftAggreg200MS::setCurrentHarmonic(const uint8_t channel, const uint8_t harmonic, const AnalyzerSoftAggreg200MS::Complex_t val) {
  pCurrent_Harmonics[channel * 50 + harmonic] = sqrt(val.Re * val.Re + val.Im * val.Im) / M_SQRT2 * 2.0;
  pCurrent_HarmonicsComplex[channel * 50 + harmonic] = val;
}
AnalyzerSoftAggreg200MS::Complex_t AnalyzerSoftAggreg200MS::VoltageHarmonicComplex(const uint8_t channel, const uint8_t harmonic) const {
  return pVoltage_HarmonicsComplex[channel * 50 + harmonic];
}
AnalyzerSoftAggreg200MS::Complex_t AnalyzerSoftAggreg200MS::CurrentHarmonicComplex(const uint8_t channel, const uint8_t harmonic) const {
  return pCurrent_HarmonicsComplex[channel * 50 + harmonic];
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
void AnalyzerSoftAggreg200MS::setVoltagePhase(const uint8_t channel, const float val) {
  pVoltage_Phase[channel] = val;
}
void AnalyzerSoftAggreg200MS::setPhi(const uint8_t channel, const float val) {
  pPhi[channel] = val;
}
void AnalyzerSoftAggreg200MS::setPowerFactor(const uint8_t channel, const float val) {
  pPower_Factor[channel] = val;
}
bool AnalyzerSoftAggreg200MS::GetSeriesJSON(const string &serie, json &result) const {
  if(AnalyzerSoftAggreg::GetSeriesJSON(serie, result) == true) return true;
  if(serie == "Voltage_Harmonics_Complex") {
    json Tmp1 = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) {
      json Tmp2 = json::array();
      for(uint8_t i = 0; i < 50; i++) {
        json Tmp3 = json::array();
        Complex_t Complex = VoltageHarmonicComplex(n, i);
        Tmp3.push_back(Complex.Re);
        Tmp3.push_back(Complex.Im);
        Tmp2.push_back(Tmp3);
      }
      Tmp1.push_back(Tmp2);
    }
    result["Voltage_Harmonics_Complex"] = Tmp1;
  } else if(serie == "Current_Harmonics_Complex") {
    json Tmp1 = json::array();
    for(uint8_t n = 0; n < pCurrentChannels; n++) {
      json Tmp2 = json::array();
      for(uint8_t i = 0; i < 50; i++) {
        json Tmp3 = json::array();
        Complex_t Complex = CurrentHarmonicComplex(n, i);
        Tmp3.push_back(Complex.Re);
        Tmp3.push_back(Complex.Im);
        Tmp2.push_back(Tmp3);
      }
      Tmp1.push_back(Tmp2);
    }
    result["Current_Harmonics_Complex"] = Tmp1;
  } else {
    return false;
  }
  return true;
}
void AnalyzerSoftAggreg200MS::HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) {
  uint16_t Pos   = 0;
  uint16_t Start = start * 2;
  uint8_t  Len   = len * 2;
  auto Append_String = [&](const string &val, const uint8_t bytes) -> bool {
    if(Pos >= Start) {
      for(uint8_t n = 0; n < bytes; n++, Pos++) {
        if(Pos >= (Start + Len)) return false;
        if(Pos >= Start) buff[Pos - Start] = (n >= val.size()) ? 0 : val.c_str()[n];
      }
    } else {
      Pos = Pos + bytes;
    }
    return (Pos <= (Start + Len));
  };
  auto Append_U32 = [&](const uint32_t val) {
    if(Pos >= Start) {
      uint32_t Val = htobe32(val);
      for(uint8_t n = 0; n < 4; n++, Pos++) {
        if(Pos >= (Start + Len)) return false;
        if(Pos >= Start) buff[Pos - Start] = ((uint8_t*)&Val)[n];
      }
    } else {
      Pos = Pos + 4;
    }
    return (Pos <= (Start + Len));
  };
  auto Append_U64 = [&](const uint64_t val) {
    if(Pos >= Start) {
      uint64_t Val = htobe64(val);
      for(uint8_t n = 0; n < 8; n++, Pos++) {
        if(Pos >= (Start + Len)) return false;
        if(Pos >= Start) buff[Pos - Start] = ((uint8_t*)&Val)[n];
      }
    } else {
      Pos = Pos + 8;
    }
    return (Pos <= (Start + Len));
  };
  auto Append_Float = [&](const float val) {
    uint32_t *Src = (uint32_t*)&val;
    if(Pos >= Start) {
      uint32_t Val = htobe32(*Src);
      for(uint8_t n = 0; n < 4; n++, Pos++) {
        if(Pos >= (Start + Len)) return false;
        if(Pos >= Start) buff[Pos - Start] = ((uint8_t*)&Val)[n];
      }
    } else {
      Pos = Pos + 4;
    }
    return (Pos <= (Start + Len));
  };
  if(Append_String(pSerial, 32) == false) return;
  if(Append_U32(Timestamp() / 1000) == false) return;
  if(Append_U64(Timestamp()) == false) return;
  if(Append_Float(Frequency()) == false) return;
  if(pVoltageChannels > 1) {
    if(Append_Float(Unbalance()) == false) return;
  }
  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) {
    if(Append_Float(Voltage(ch)) == false) return;
  }
  for(uint8_t ch = 0; ch < pCurrentChannels; ch++) {
    if(Append_Float(Current(ch)) == false) return;
  }
  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) {
    if(Append_Float(ActivePower(ch)) == false) return;
  }
  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) {
    if(Append_Float(ReactivePower(ch)) == false) return;
  }
  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) {
    if(Append_Float(ActiveEnergy(ch)) == false) return;
  }
  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) {
    if(Append_Float(ReactiveEnergy(ch)) == false) return;
  }
  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) {
    if(Append_Float(ApparentPower(ch)) == false) return;
  }
  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) {
    if(Append_Float(PowerFactor(ch)) == false) return;
  }
  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) {
    if(Append_Float(VoltageTHD(ch)) == false) return;
  }
  for(uint8_t ch = 0; ch < pCurrentChannels; ch++) {
    if(Append_Float(CurrentTHD(ch)) == false) return;
  }
  if(pVoltageChannels > 1) {
    for(uint8_t ch = 0; ch < pVoltageChannels; ch++) {
      if(Append_Float(VoltagePhase(ch)) == false) return;
    }
  }
  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) {
    if(Append_Float(Phi(ch)) == false) return;
  }
  for(uint8_t h = 0; h < 50; h++) {
    for(uint8_t ch = 0; ch < pVoltageChannels; ch++) {
      if(Append_Float(VoltageHarmonic(ch, h)) == false) return;
    }
  }
  for(uint8_t h = 0; h < 50; h++) {
    for(uint8_t ch = 0; ch < pCurrentChannels; ch++) {
      if(Append_Float(CurrentHarmonic(ch, h)) == false) return;
    }
  }
  for(uint8_t h = 0; h < 50; h++) {
    for(uint8_t ch = 0; ch < pVoltageChannels; ch++) {
      if(Append_Float(PowerHarmonic(ch, h)) == false) return;
    }
  }
}
// ---------------------------------------------------------------------------------------------------------------------