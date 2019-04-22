/* ================================== zMeter ===========================================================================
 * File:     TariffPlugin_NONE.h
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "../TariffPlugin.h"
class TariffPower_NONE : public TariffPowerPlugin {
  private :
    static bool pRegistered;
  private :
    float pContracted;
    float pCost;
  private :
    static TariffPowerPlugin *Instance();
  public :
    ~TariffPower_NONE();
    void  Config(const float contracted, const float cost);
    void  PushSample(const float active, const float reactive);
    float PopSample(const float active, const float reactive);
};
//----------------------------------------------------------------------------------------------------------------------
class TariffEnergy_NONE : public TariffEnergyPlugin {
  private :
    static bool pRegistered;
  private :
    static TariffEnergyPlugin *Instance();
  private :
    float pCost;
  public :
    ~TariffEnergy_NONE();
    void  Config(const float cost);
    void  PushSample(const float active, const float reactive);
    float PopSample(const float active, const float reactive);
};
//----------------------------------------------------------------------------------------------------------------------