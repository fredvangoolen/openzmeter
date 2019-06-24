/* ================================== zMeter ===========================================================================
 * File:     Spain.h
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "../Tariff.h"
class TariffPower_ESP3 : public TariffPowerPlugin {
  private :
    static bool pRegistered;
  private :
    uint32_t pLen;
    float    pMaxPower;
  private :
    static TariffPowerPlugin *Instance();
  public :
    ~TariffPower_ESP3();
    void         Config(const float contracted, const float cost);
    CycleStats_t CycleStats();
    void         PushSample(const float active, const float reactive);
    float        PopSample(const float active, const float reactive);
};
class TariffPower_ESP6 : public TariffPowerPlugin {
  private :
    static bool pRegistered1;
    static bool pRegistered2;
    static bool pRegistered345;
    static bool pRegistered6;
  private :
    uint32_t pLen;
    float    pConst;
    uint32_t pExcessQuarters;
    float    pAccum;
  private :
    static TariffPowerPlugin *Instance1();
    static TariffPowerPlugin *Instance2();
    static TariffPowerPlugin *Instance345();
    static TariffPowerPlugin *Instance6();
  public :
    ~TariffPower_ESP6();
    void         Config(const float contracted, const float cost);
    CycleStats_t CycleStats();
    void         PushSample(const float active, const float reactive);
    float        PopSample(const float active, const float reactive);
};
//----------------------------------------------------------------------------------------------------------------------
class TariffEnergy_ESP : public TariffEnergyPlugin {
  private :
    static bool pRegistered;
  private :
    static TariffEnergyPlugin *Instance();
  private :
    uint32_t pLen;
    float    pReactive;
  public :
    ~TariffEnergy_ESP();
    void         Config(const float cost);
    CycleStats_t CycleStats();
    void         PushSample(const float active, const float reactive);
    float        PopSample(const float active, const float reactive);
};
//----------------------------------------------------------------------------------------------------------------------