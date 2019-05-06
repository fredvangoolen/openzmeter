#pragma once
#include <string>
#include <dbus/dbus.h>
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class Bluetooth {
  private :
    DBusConnection *pDBus;
    int             pBridgeSocket;
  public:
    Bluetooth();
    virtual ~Bluetooth();
  private:
    string GetMac();
    void   SetDiscoverable(bool value, uint32_t timeout);
    void   SetPairable(bool value, uint32_t timeout);
    void   SetName(string name);
    void   SetEnabled(bool value);
    void   SetClass(uint32_t dev_class);
    void   SendBool(const char *property, bool value);
    void   SendUINT32(const char *property, uint32_t value);
    void   SendString(const char *property, const char *value);
    bool   AddBridge(const char *name);
};