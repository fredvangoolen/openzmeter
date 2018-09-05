/* ================================== zMeter ===========================================================================
 * File:     DeviceRemote.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <libusb-1.0/libusb.h>
#include "DeviceInterface.h"
#include "../Analyzers/AnalyzerRemote.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
class DeviceRemote : public DeviceInterface {
  private :
    typedef struct {
      char  *Buffer;
      size_t Size;
      size_t MaxSize;
    } CurlBuff_t;
  private :
    char                        *pSerial;                  //Serial of device to sync
    char                        *pServer;                  //Server to sync
    char                        *pUserName;                //Server user
    char                        *pPassword;                //Server password
    CurlBuff_t                   pCurlBuff;                //Buffer for curl responses
    AnalyzerRemote             **pAnalyzers;               //Analyzers list
    uint8_t                      pAnalyzersCount;          //Analyzers count
  private :
    AnalyzerInterface *FindAnalyzer(char *serial);
  private :
    static void  *Capture_Thread_Client(void *args);
    static void  *Capture_Thread_Server(void *args);
    static size_t Curl_Read_Callback(void *contents, size_t size, size_t nmemb, void *userp);
    static bool   SetsetRemoteAnalyzerCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
//    static void   ReadCallback(struct libusb_transfer *transfer);
//    static int    PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
//  private :
//    bool ReadJSONFloatArray(json_object *object, float *dst, uint8_t len);
//    bool ReadOutput(json_object *object, bool parseOnly);
  public :
    DeviceRemote(json_object* config);
    ~DeviceRemote();
};
//----------------------------------------------------------------------------------------------------------------------
