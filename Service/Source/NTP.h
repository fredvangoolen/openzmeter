#pragma once
#include "Tools.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
class NTP {
  private : 
    typedef struct {
      uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
      uint8_t stratum;         // Eight bits. Stratum level of the local clock.
      uint8_t poll;            // Eight bits. Maximum interval between successive messages.
      uint8_t precision;       // Eight bits. Precision of the local clock.
      uint32_t rootDelay;      // 32 bits. Total round trip delay time.
      uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
      uint32_t refId;          // 32 bits. Reference clock identifier.
      uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
      uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.
      uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
      uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.
      uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
      uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.
      uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
      uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.
    } NTP_Packet;              // Total: 384 bits or 48 bytes.
  private :  
    static NTP *pInstance;
  private :
    bool            pTerminate;
    pthread_t       pThread;
    pthread_mutex_t pMutex;
    bool            pEnabled;
    string          pServer;
    json            pConfig;
  private :
    static void *NTP_Thread(void *args);
    static void GetTimeSettingsCallback(HTTPRequest &req);
    static void SetTimeSettingsCallback(HTTPRequest &req);
  public :
    NTP(json &config);
    virtual ~NTP();
    NTP *Instance();
    void Stop();
    void Start();
};