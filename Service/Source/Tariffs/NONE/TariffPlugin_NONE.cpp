/* ================================== zMeter ===========================================================================
 * File:     TariffPlugin_NONE.cpp
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "TariffPlugin_NONE.h"
bool TariffPower_NONE::pRegistered = TariffPower_NONE::Register("NONE", TariffPower_NONE::Instance);
TariffPowerPlugin *TariffPower_NONE::Instance(float contracted, float cost) {
  TariffPower_NONE *Instance = new TariffPower_NONE;
  Instance->pContracted = contracted;
  Instance->pCost = cost;
  return Instance;
}
TariffPower_NONE::~TariffPower_NONE() {
}
void TariffPower_NONE::Reset() {
}
void TariffPower_NONE::PushSample(const float active, const float reactive) {
}
float TariffPower_NONE::PopSample(const float active, const float reactive) { 
  return (pContracted * pCost) / 96.0;
}
//----------------------------------------------------------------------------------------------------------------------
bool TariffEnergy_NONE::pRegistered = TariffEnergy_NONE::Register("NONE", TariffEnergy_NONE::Instance);
TariffEnergyPlugin *TariffEnergy_NONE::Instance(const float cost) {
  TariffEnergy_NONE *Instance = new TariffEnergy_NONE;
  Instance->pCost = cost;
  return Instance;
}
TariffEnergy_NONE::~TariffEnergy_NONE() {
}
void TariffEnergy_NONE::Reset() {
}
void TariffEnergy_NONE::PushSample(const float active, const float reactive) {
}
float TariffEnergy_NONE::PopSample(const float active, const float reactive) {
  return (active / 4.0) * pCost;
}
//----------------------------------------------------------------------------------------------------------------------

