/* ================================== zMeter ===========================================================================
 * File:     TariffPlugin.h
 * Author:   Eduardo Viciana (2019)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <string>
#include <vector>
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class TariffPowerPlugin {
  private :
    typedef TariffPowerPlugin* (*Func_t)(float contracted, float cost);
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
    static TariffPowerPlugin *Instance(string name, float contracted, float cost);
  public :
    virtual ~TariffPowerPlugin();
    virtual void  Reset() = 0;
    virtual void  PushSample(float active, float reactive) = 0;
    virtual float PopSample() = 0;
};
//----------------------------------------------------------------------------------------------------------------------
class TariffReactivePlugin {
  private :
    typedef TariffReactivePlugin* (*Func_t)();
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
    static TariffReactivePlugin *Instance(string name);
  public :
    virtual ~TariffReactivePlugin();
    virtual void  Reset() = 0;
    virtual void  PushSample(float) = 0;
    virtual float PopSample() = 0;
};
//----------------------------------------------------------------------------------------------------------------------