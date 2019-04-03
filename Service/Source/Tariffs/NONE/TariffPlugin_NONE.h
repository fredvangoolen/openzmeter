/* ================================== zMeter ===========================================================================
 * File:     TariffPlugin_NONE.h
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "../TariffPlugin.h"
class TariffActive_NONE : public TariffPowerPlugin {
  private :
    static bool pRegistered;
  private :
    float pContracted;
    float pCost;
  private :
    static TariffPowerPlugin *Instance(float contracted, float cost);
  public :
    ~TariffActive_NONE();
    void  Reset();
    void  PushSample(float);
    float PopSample();
};
//----------------------------------------------------------------------------------------------------------------------
class TariffReactive_NONE : public TariffReactivePlugin {
  private :
    static bool pRegistered;
  private :
    static TariffReactivePlugin *Instance();
  public :
    ~TariffReactive_NONE();
    void  Reset();
    void  PushSample(float);
    float PopSample();
};
//----------------------------------------------------------------------------------------------------------------------