#pragma once
#ifdef OPENWRT
  #include "OpenWRT.h"
#else
  #include "Debian.h"
#endif

class SysSupport {
  private :
    static bool Registered;
  private :
    static bool Register();
    static void GetNewsCallback(HTTPRequest &req);
    static void GetUsersCallback(HTTPRequest &req);
    static void GetUserCallback(HTTPRequest &req);
    static void SetUserCallback(HTTPRequest &req);
};