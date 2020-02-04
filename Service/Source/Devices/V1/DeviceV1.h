#pragma once
#include <libusb-1.0/libusb.h>
#include <deque>
#include "Device.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
#define DEVICEV1_VID             0x0483                          //Product ID
#define DEVICEV1_PID             0x7270                          //Device ID
#define DEVICEV1_MANSTRING       "zRed"                          //Manufacturer string
#define DEVICEV1_01_PRODSTRING   "openZmeter - Capture device"   //Product string
#define DEVICEV1_ENDPOINT_IN       0x81                          //Endpoint de lectura de datos
#define DEVICEV1_SAMPLEFREQ    15625.00                          //Frequencia de muestreo
#define DEVICEV1_BUFFERSECONDS       10                          //Tamaño del buffer (10 segundos)
#define DEVICEV1_TRANSFERS           16                          //Numero de transferencias a encolar
#define DEVICEV1_BUFFERSIZE    0x040000
#define DEVICEV1_VOLTAGEDELAY         2                          //Samples to delay
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class DeviceV1 final : public Device {
  private :
    class Transfer {
      private :
        libusb_transfer *pTransfer;
        uint8_t         *pBuffer;
        uint32_t         pBufferSize;
        DeviceV1        *pParent;
      public :
        Transfer();
        void Init(DeviceV1 *parent, uint32_t size);
        ~Transfer();
        int Submit();
        int Submit(libusb_device_handle *deviceHandle, uint8_t endpoint, libusb_transfer_cb_fn callback, int32_t timeout);
        uint32_t BufferSize();
    };
  private :
    static bool                  pRegistered;
  private :
    bool                         pEnabled;                 //Enable device
    uint8_t                      pSubmittedTransfers;      //Keep count of currently submitted transfers
    pthread_t                    pThread;                  //Descriptor del hilo de captura
    bool                         pTerminate;               //Marca que deben detenerse los trabajos pendientes
    int16_t                     *pBuffer;                  //Read buffer
    uint32_t                     pBufferTail;              //pBuffer read pos
    uint32_t                     pBufferHead;              //pBuffer write pos
    pthread_mutex_t              pConfigLock;              //Block configuration params
    float                        pVoltageGain;             //Voltage gain
    float                        pCurrentGain;             //Current gain
    float                        pVoltageOffset;           //Voltage offset
    float                        pCurrentOffset;           //Current offset
    bool                         pCurrentInvert;           //Invert current channel
    float                        pVoltageMult;             //Voltage gain depens on device version (mounted voltage divisor)
    struct libusb_device_handle *pDeviceHandle;            //USB handle
    struct libusb_device        *pNewDevice;               //USB device
    libusb_context              *pContext;                 //USB context
    int32_t                      pRunAverageValues[2];     //Average wave value of last second
    uint32_t                     pRunAverageCounter;       //Somple count for average
    float                        pAverageValues[2];        //Average wave value of last second
    pthread_rwlock_t             pAverageLocks;            //Lock access to average values
    AnalyzerSoft                *pAnalyzer;
    uint8_t                      pVoltageDelayPos;
    Transfer                     pTransfers[DEVICEV1_TRANSFERS];
    float                        pVoltageDelay[DEVICEV1_VOLTAGEDELAY];
  private :
    static bool  Register();
    static vector<Device*> GetInstances();
    static void *Capture_Thread(void *args);
    static void  ReadCallback(struct libusb_transfer *transfer);
    static int   PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
    static json  CheckConfig(const json &config);
  private :
    DeviceV1(const string &serial, const uint16_t version);
    void Configure(const json &config);
    void GetDevice(const json &params, Response &response) override;
    void SetDevice(const json &params, Response &response) override;
    void BufferAppend(int16_t *sample);
  public :
    ~DeviceV1();
};
//----------------------------------------------------------------------------------------------------------------------