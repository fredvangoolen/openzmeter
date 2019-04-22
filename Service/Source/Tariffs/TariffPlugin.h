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
    typedef TariffPowerPlugin* (*Func_t)();
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
    static TariffPowerPlugin *Instance(const string name);
  public :
    string pName;
  public :
    string Name();
  public :
    virtual ~TariffPowerPlugin();
    virtual void  Config(const float contracted, const float cost) = 0;
    virtual void  PushSample(const float active, const float reactive) = 0;
    virtual float PopSample(const float active, const float reactive) = 0;
};
//----------------------------------------------------------------------------------------------------------------------
class TariffEnergyPlugin {
  private :
    typedef TariffEnergyPlugin* (*Func_t)();
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
    static TariffEnergyPlugin *Instance(string name);
  public :
    string pName;
  public :
    string Name();
  public :
    virtual ~TariffEnergyPlugin();
    virtual void  Config(const float cost) = 0;
    virtual void  PushSample(const float active, const float reactive) = 0;
    virtual float PopSample(const float active, const float reactive) = 0;
};
//----------------------------------------------------------------------------------------------------------------------