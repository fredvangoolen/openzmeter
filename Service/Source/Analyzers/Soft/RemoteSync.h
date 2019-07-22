/* ================================== zMeter ===========================================================================
 * File:     RemoteSync.h
 * Author:   Eduardo Viciana (agosto de 2019)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
//#include <curl/curl.h>

class RemoteSync : public Database {
  public:
    RemoteSync(string serial, string server, string username, string password);
  virtual ~RemoteSync();
private:

};