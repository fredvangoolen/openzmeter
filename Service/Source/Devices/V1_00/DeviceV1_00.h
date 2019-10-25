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
#define DEVICEV1_00_VID             0x0483                          //Product ID
#define DEVICEV1_00_PID             0x7270                          //Device ID
#define DEVICEV1_00_VERSION         0x0100                          //Device version
#define DEVICEV1_00_MANSTRING       "zRed"                          //Manufacturer string
#define DEVICEV1_00_PRODSTRING      "openZmeter - Capture device"   //Product string
#define DEVICEV1_00_ENDPOINT_IN       0x81                          //Endpoint de lectura de datos
#define DEVICEV1_00_SAMPLEFREQ    15625.00                          //Frequencia de muestreo
#define DEVICEV1_00_BUFFERSECONDS       10                          //Tamaño del buffer (10 segundos)
#define DEVICEV1_00_TRANSFERS           32                          //Numero de transferencias a encolar
#define DEVICEV1_00_VOLTAGEDELAY         2                          //Samples to delay
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class DeviceV1_00 : public Device {
  private :
    typedef struct {
      float Voltage;
      float Current;
    } Sample_t;
    typedef struct {
      uint8_t          Buffer[8192];
      libusb_transfer *Transfer;
    } Transfer_t;   
  private :
    static bool                  pRegistered;
  private :
    pthread_t                    pCaptureThread;           //Descriptor del hilo de captura
    pthread_mutex_t              pMutex;                   //Critical sections locks
    bool                         pTerminate;               //Marca que deben detenerse los trabajos pendientes
    Transfer_t                   pTransfers[DEVICEV1_00_TRANSFERS];
    deque<Sample_t>              pBuffer;                  //Read buffer
    float                        pVoltageGain;             //Voltage gain
    float                        pCurrentGain;             //Current gain
    float                        pVoltageOffset;           //Voltage offset
    float                        pCurrentOffset;           //Current offset
    bool                         pCurrentInvert;           //Invert current channel
    struct libusb_device_handle *pDeviceHandle;            //USB handle
    struct libusb_device        *pNewDevice;               //USB device
    libusb_context              *pContext;                 //USB context
    pthread_t                    pSignalThread;            //Thread to handle LED and sounds
    deque<float>                 pDelay;                   //Delay buffer for voltage/current compensation
    deque<Sample_t>              pAverageValues;           //Average wave value of last second
  private :
    static bool  GetInstances();
    static void *Capture_Thread(void *args);
    static void *Sig_Thread(void *args);
    static void  ReadCallback(struct libusb_transfer *transfer);
    static int   PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
    static json  CheckConfig(const json &config);
  private :
    DeviceV1_00(string serial);
    bool Start(const json &config);
    bool StartDevice() override;
    bool StopDevice() override;
    void GetDevice(HTTPRequest &req) override;
    void SetDevice(HTTPRequest &req) override;
    bool BuffersAppend(float voltageSample, float currentSample);
  public :
    ~DeviceV1_00();
};
//----------------------------------------------------------------------------------------------------------------------
