/* ================================== zMeter ===========================================================================
 * File:     TariffPlugin_NONE.cpp
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "TariffPlugin_NONE.h"
bool TariffActive_NONE::pRegistered = TariffActive_NONE::Register("NONE", TariffActive_NONE::Instance);
TariffPowerPlugin *TariffActive_NONE::Instance(float contracted, float cost) {
  TariffActive_NONE *Instance = new TariffActive_NONE;
  Instance->pContracted = contracted;
  Instance->pCost = cost;
  return new TariffActive_NONE;
}
TariffActive_NONE::~TariffActive_NONE() {
}
void TariffActive_NONE::Reset() {
}
void TariffActive_NONE::PushSample(float val) {
}
float TariffActive_NONE::PopSample() {
  return (pContracted * pCost) / 96.0;
}
//----------------------------------------------------------------------------------------------------------------------
bool TariffReactive_NONE::pRegistered = TariffReactive_NONE::Register("NONE", TariffReactive_NONE::Instance);
TariffReactivePlugin *TariffReactive_NONE::Instance() {
  return new TariffReactive_NONE;
}
TariffReactive_NONE::~TariffReactive_NONE() {
}
void TariffReactive_NONE::Reset() {
}
void TariffReactive_NONE::PushSample(float val) {
}
float TariffReactive_NONE::PopSample() {
  return 0.0;
}
//----------------------------------------------------------------------------------------------------------------------

