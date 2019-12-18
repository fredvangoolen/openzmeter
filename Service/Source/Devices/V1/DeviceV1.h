/* ================================== zMeter ===========================================================================
 * File:     CaptureV1.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
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
#define DEVICEV1_TRANSFERS           32                          //Numero de transferencias a encolar
#define DEVICEV1_VOLTAGEDELAY         2                          //Samples to delay
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class DeviceV1 final : public Device {
  private :
    typedef struct {
      float Voltage;
      float Current;
    } Sample_t;
    class Transfer {
      private :
        libusb_transfer *pTransfer;
        uint8_t         *pBuffer;
        uint32_t         pBufferSize;
        DeviceV1        *pParent;
      public :
        Transfer(DeviceV1 *parent, uint32_t size);
        ~Transfer();
        int Submit();
        int Submit(libusb_device_handle *deviceHandle, uint8_t endpoint, libusb_transfer_cb_fn callback, int32_t timeout);
        uint32_t BufferSize();
    };
  private :
    static bool                  pRegistered;
  private :
    float                        pVoltageMult;             //Voltage gain depens on device version (mounted voltage divisor)
    uint8_t                      pSubmittedTransfers;      //Keep count of currently submitted transfers
    uint16_t                     pHWVersion;               //Hardware version
    pthread_t                    pCaptureThread;           //Descriptor del hilo de captura
    bool                         pTerminate;               //Marca que deben detenerse los trabajos pendientes
    deque<Sample_t>              pBuffer;                  //Read buffer
    float                        pVoltageGain;             //Voltage gain
    float                        pCurrentGain;             //Current gain
    float                        pVoltageOffset;           //Voltage offset
    float                        pCurrentOffset;           //Current offset
    bool                         pCurrentInvert;           //Invert current channel
    struct libusb_device_handle *pDeviceHandle;            //USB handle
    struct libusb_device        *pNewDevice;               //USB device
    libusb_context              *pContext;                 //USB context
    deque<float>                 pDelay;                   //Delay buffer for voltage/current compensation
    deque<Sample_t>              pAverageValues;           //Average wave value of last second
    RWLocks                      pAverageLocks;            //Lock access to average values
    vector<Transfer>             pTransfers;
  private : 
    static bool  GetInstances();
    static void *Capture_Thread(void *args);
    static void  ReadCallback(struct libusb_transfer *transfer);
    static int   PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
    static json  CheckConfig(const json &config);
  private :
    DeviceV1(const string &serial, const uint16_t version);
    bool StartDevice() override;
    bool StopDevice() override;
    void GetDevice(HTTPRequest &req) override;
    void SetDevice(HTTPRequest &req) override;
    bool BuffersAppend(float voltageSample, float currentSample);
  public :
    ~DeviceV1();
};
//----------------------------------------------------------------------------------------------------------------------
