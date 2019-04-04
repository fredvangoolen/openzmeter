/* ================================== zMeter ===========================================================================
 * File:     TariffPlugin.h
 * Author:   Eduardo Viciana (2019)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Tariff.h"
#include <string>
#include <vector>
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class TariffPowerPlugin {
  private :
    typedef TariffPowerPlugin* (*Func_t)(const float contracted, const float cost);
    typedef struct {
      string Name;
      Func_t Instance;
    } Type_t;
  private :
    static vector<Type_t> pPlugins;
  protected :
    static bool Register(const string name, const Func_t instance);
  public :
    static bool Contains(const string name);
    static TariffPowerPlugin *Instance(const string name, const float contracted, const float cost);
  public :
    virtual ~TariffPowerPlugin();
    virtual void  Reset() = 0;
    virtual void  PushSample(const float active, const float reactive) = 0;
    virtual float PopSample(const float active, const float reactive) = 0;
};
//----------------------------------------------------------------------------------------------------------------------
class TariffEnergyPlugin {
  private :
    typedef TariffEnergyPlugin* (*Func_t)(float cost);
    typedef struct {
      string Name;
      Func_t Instance;
    } Type_t;
  private :
    static vector<Type_t> pPlugins;
  protected :
    static bool Register(string name, Func_t instance);
  public :
    static bool Contains(string name);
    static TariffEnergyPlugin *Instance(string name, float cost);
  public :
    virtual ~TariffEnergyPlugin();
    virtual void  Reset() = 0;
    virtual void  PushSample(const float active, const float reactive) = 0;
    virtual float PopSample(const float active, const float reactive) = 0;
};
//----------------------------------------------------------------------------------------------------------------------