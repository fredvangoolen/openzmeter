/* ================================== zMeter ===========================================================================
 * File:     TariffPlugin_ESP.cpp
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <iomanip>
#include "TariffPlugin_ESP.h"
bool TariffPower_ESP3::pRegistered = TariffPower_ESP3::Register("ESP_3.x", TariffPower_ESP3::Instance);
TariffPowerPlugin *TariffPower_ESP3::Instance() {
  TariffPower_ESP3 *Module = new TariffPower_ESP3();
  Module->pLen = 0;
  Module->pMaxPower = 0;
  return Module;
}
TariffPower_ESP3::~TariffPower_ESP3() {
}
void TariffPower_ESP3::Config(const float contracted, const float cost) {
  pStats.Contracted = contracted;
  pStats.Cost = cost;
}
void TariffPower_ESP3::PushSample(const float active, const float reactive) {
  pLen ++;
  if(active > pMaxPower) pMaxPower = active;
}
float TariffPower_ESP3::PopSample(const float active, const float reactive) { 
  return (pStats.Contracted * pStats.Cost) / 96.0;
}
TariffPower_ESP3::CycleStats_t TariffPower_ESP3::CycleStats() {
  pStats.Days = pLen / 96;
  if(pMaxPower < (0.85 * pStats.Contracted)) {
    std::ostringstream Formatter;
    Formatter << std::fixed;
    Formatter << std::setprecision(2);
    Formatter << (pMaxPower / 1000.0);
    pStats.Other = "Contracted power bonus (Max power: " + Formatter.str() + "kW)";
    pStats.OtherCosts = -0.15 * pStats.Contracted * pStats.Days * pStats.Cost;
  } else if((pMaxPower >= (0.85 * pStats.Contracted)) && (pMaxPower <= (1.05 * pStats.Contracted))) {
    pStats.Other = "";
    pStats.OtherCosts = 0.0;
  } else if(pMaxPower > (1.05 * pStats.Contracted)) {
    std::ostringstream Formatter;
    Formatter << std::fixed;
    Formatter << std::setprecision(2);
    Formatter << (pMaxPower / 1000.0);
    pStats.Other = "Contracter power penalty (Max power: " + Formatter.str() + "kW)";
    pStats.OtherCosts = (2 * (pMaxPower - 1.05 * pStats.Contracted)) * pStats.Cost * pStats.Days;
  }
  pLen = 0;
  return pStats;
}
//----------------------------------------------------------------------------------------------------------------------
bool TariffPower_ESP6::pRegistered = TariffPower_ESP6::Register("ESP_6.x", TariffPower_ESP6::Instance);
TariffPowerPlugin *TariffPower_ESP6::Instance() {
  TariffPower_ESP6 *Module = new TariffPower_ESP6();
  Module->pLen = 0;
  return Module;
}
TariffPower_ESP6::~TariffPower_ESP6() {
}
void TariffPower_ESP6::Config(const float contracted, const float cost) {
  pStats.Contracted = contracted;
  pStats.Cost = cost;
}
void TariffPower_ESP6::PushSample(const float active, const float reactive) {
  pLen ++;
}
float TariffPower_ESP6::PopSample(const float active, const float reactive) { 
  return (pStats.Contracted * pStats.Cost) / 96.0;
}
TariffPower_ESP6::CycleStats_t TariffPower_ESP6::CycleStats() {
  pStats.Other = "";
  pStats.OtherCosts = 0.0;
  pStats.Days = pLen / 96;
  pLen = 0;
  return pStats;
}
//----------------------------------------------------------------------------------------------------------------------
bool TariffEnergy_ESP::pRegistered = TariffEnergy_ESP::Register("NONE", TariffEnergy_ESP::Instance);
TariffEnergyPlugin *TariffEnergy_ESP::Instance() {
  TariffEnergy_ESP *Module = new TariffEnergy_ESP();
  Module->pLen = 0;
  return Module;
}
TariffEnergy_ESP::~TariffEnergy_ESP() {
}
void TariffEnergy_ESP::Config(const float cost) {
  pStats.Cost = cost;
}
void TariffEnergy_ESP::PushSample(const float active, const float reactive) {
  if(pLen == 0) {
    pStats.Consumed = 0.0;
    pReactive = 0.0;
  }
  pLen++;
  pStats.Consumed += active / 4.0;
}
float TariffEnergy_ESP::PopSample(const float active, const float reactive) {
  return (active / 4.0) * pStats.Cost;
}
TariffEnergy_ESP::CycleStats_t TariffEnergy_ESP::CycleStats() {
  float Cos = pStats.Consumed / sqrt(pow(pStats.Consumed, 2.0) + pow(pReactive, 2.0));
  float Excess = pReactive - 0.33 * pStats.Consumed; 
  if(Cos >= 0.95) {
    pStats.Other = "";
    pStats.OtherCosts = 0.0;
  } else if((Cos < 0.95) && (Cos >= 0.8)) {
    std::ostringstream Formatter;
    Formatter << std::fixed << std::setprecision(2) << Cos << "  " << (Excess / 1000.0);
    pStats.Other = "Reactive energy (PF: " + Formatter.str() + "kWh)";
    pStats.OtherCosts = pReactive * 0.041554 / 1000.0;
  } else if(Cos < 0.8) {
    std::ostringstream Formatter;
    Formatter << std::fixed << std::setprecision(2) << Cos << "  " << (Excess / 1000.0);
    pStats.Other = "Reactive energy (PF: " + Formatter.str() + "kWh)";
    pStats.OtherCosts = pReactive * 0.062332 / 1000.0;
  }
  pLen = 0;
  return pStats;
}
//----------------------------------------------------------------------------------------------------------------------