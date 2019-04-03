#include "TariffPlugin.h"
//----------------------------------------------------------------------------------------------------------------------
vector<TariffPowerPlugin::Type_t> TariffPowerPlugin::pPlugins;
bool TariffPowerPlugin::Register(string name, Func_t instance) {
  Type_t Type;
  Type.Name = name;
  Type.Instance = instance;
  pPlugins.push_back(Type);
  return true;
}
bool TariffPowerPlugin::Contains(string name) {
  for(vector<Type_t>::iterator it = pPlugins.begin() ; it != pPlugins.end(); ++it) {
    if((*it).Name == name) return true;
  }
  return false;
}
TariffPowerPlugin *TariffPowerPlugin::Instance(string name, float contracted, float cost) {
  for(vector<Type_t>::iterator it = pPlugins.begin() ; it != pPlugins.end(); ++it) {
    if((*it).Name == name) return (*it).Instance(contracted, cost);
  }
  return NULL;
}
TariffPowerPlugin::~TariffPowerPlugin() {
}
//----------------------------------------------------------------------------------------------------------------------
vector<TariffReactivePlugin::Type_t> TariffReactivePlugin::pPlugins;
bool TariffReactivePlugin::Register(string name, Func_t instance) {
  Type_t Type;
  Type.Name = name;
  Type.Instance = instance;
  pPlugins.push_back(Type);
  return true;
}
bool TariffReactivePlugin::Contains(string name) {
  for(vector<Type_t>::iterator it = pPlugins.begin() ; it != pPlugins.end(); ++it) {
    if((*it).Name == name) return true;
  }
  return false;
}
TariffReactivePlugin *TariffReactivePlugin::Instance(string name) {
  for(vector<Type_t>::iterator it = pPlugins.begin() ; it != pPlugins.end(); ++it) {
    if((*it).Name == name) return (*it).Instance();
  }
  return NULL;
}
TariffReactivePlugin::~TariffReactivePlugin() {
}
