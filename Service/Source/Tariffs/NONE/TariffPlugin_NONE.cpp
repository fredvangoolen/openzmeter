/* ================================== zMeter ===========================================================================
 * File:     TariffPlugin_NONE.cpp
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "TariffPlugin_NONE.h"
bool TariffPower_NONE::pRegistered = TariffPower_NONE::Register("NONE", TariffPower_NONE::Instance);
TariffPowerPlugin *TariffPower_NONE::Instance() {
  TariffPower_NONE *Module = new TariffPower_NONE();
  Module->pLen = 0;
  return Module;
}
TariffPower_NONE::~TariffPower_NONE() {
}
void TariffPower_NONE::Config(const float contracted, const float cost) {
  pStats.Contracted = contracted;
  pStats.Cost = cost;
}
void TariffPower_NONE::PushSample(const float active, const float reactive) {
  pLen ++;
}
float TariffPower_NONE::PopSample(const float active, const float reactive) { 
  return (pStats.Contracted * pStats.Cost) / 96.0;
}
TariffPower_NONE::CycleStats_t TariffPower_NONE::CycleStats() {
  pStats.Other = "";
  pStats.OtherCosts = 0.0;
  pStats.Days = pLen / 96;
  pStats.ContractedCosts = pStats.Cost * pStats.Contracted * pStats.Days;
  pStats.Total = pStats.ContractedCosts + pStats.OtherCosts;
  pLen = 0;
  return pStats;
}
//----------------------------------------------------------------------------------------------------------------------
bool TariffEnergy_NONE::pRegistered = TariffEnergy_NONE::Register("NONE", TariffEnergy_NONE::Instance);
TariffEnergyPlugin *TariffEnergy_NONE::Instance() {
  TariffEnergy_NONE *Module = new TariffEnergy_NONE();
  Module->pLen = 0;
  return Module;
}
TariffEnergy_NONE::~TariffEnergy_NONE() {
}
void TariffEnergy_NONE::Config(const float cost) {
  pCost = cost;
}
void TariffEnergy_NONE::PushSample(const float active, const float reactive) {
}
float TariffEnergy_NONE::PopSample(const float active, const float reactive) {
  return (active / 4.0) * pCost;
}
TariffEnergy_NONE::CycleStats_t TariffEnergy_NONE::CycleStats() {
  return pStats;
}
//----------------------------------------------------------------------------------------------------------------------

