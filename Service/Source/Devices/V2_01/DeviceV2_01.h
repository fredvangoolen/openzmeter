/* ================================== zMeter ===========================================================================
 * File:     DeviceV2.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <libusb-1.0/libusb.h>
#include <deque>
#include "Device.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
#define DEVICEV2_01_VID              0x0483                          //Product ID
#define DEVICEV2_01_PID              0x7270                          //Device ID
#define DEVICEV2_01_VERSION         0x0201                          //Device version
#define DEVICEV2_01_MANSTRING       "zRed"                          //Manufacturer string
#define DEVICEV2_01_PRODSTRING      "openZmeter - Capture device"   //Product string
#define DEVICEV2_01_ENDPOINT_IN       0x81                          //Endpoint de lectura de datos
#define DEVICEV2_01_SAMPLEFREQ    19200.00                          //Frequencia de muestreo
#define DEVICEV2_01_TRANSFERS           32                          //Numero de transferencias a encolar
//----------------------------------------------------------------------------------------------------------------------
class DeviceV2_01 : public Device {
  private :
    typedef struct {
      uint8_t                          OffsetV;
      uint8_t                          OffsetC;
      Analyzer::AnalyzerMode  Mode;
      AnalyzerSoft                    *Analyzer;
    } Analyzer_t;
    typedef struct {
      float Voltage[3];
      float Current[4];
    } Sample_t;
    typedef struct {
      uint8_t          Buffer[8192];
      libusb_transfer *Transfer;
    } Transfer_t;    
  private :
    static bool                  pRegistered;    
  private :
    pthread_t                    pCaptureThread;           //Descriptor del hilo de captura
    Transfer_t                   pTransfers[DEVICEV2_01_TRANSFERS];
    bool                         pTerminate;               //Marca que deben detenerse los trabajos pendientes
    vector<Analyzer_t>           pAnalyzers;               //Listado de analyzadores
    deque<Sample_t>              pBuffer;                  //Read buffer
    vector<float>                pVoltageGain;             //Ganancia de tension
    vector<float>                pCurrentGain;             //Ganancia de corriente
    vector<float>                pVoltageOffset;           //Offset de tension
    vector<float>                pCurrentOffset;           //Offset de corriente
    Sample_t                     pCapture;                 //Capture status
    int                          pCaptureSync;             //Flag for detect lost samples
    struct libusb_device_handle *pDeviceHandle;            //Dispositivo de captura
    struct libusb_device        *pNewDevice;               //Dispositivo que se conecta
    libusb_context              *pContext;                 //USB context
    pthread_mutex_t              pMutex;
  private :
    static bool             GetInstances();
    static void            *Capture_Thread(void *args);
    static void             ReadCallback(struct libusb_transfer *transfer);
    static int              PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
  private :
    bool               SetConfig(json config);
    Analyzer *FindAnalyzer(string serial);
    bool               Start();
    bool               Stop();
    bool               BuffersAppend();
  public :
    uint32_t           AnalyzersCount();
    DeviceV2_01(string serial);
    ~DeviceV2_01();
};
//----------------------------------------------------------------------------------------------------------------------
