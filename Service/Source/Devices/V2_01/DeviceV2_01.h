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
#define DEVICEV2_01_VID             0x0483                          //Product ID
#define DEVICEV2_01_PID             0x7270                          //Device ID
#define DEVICEV2_01_VERSION         0x0201                          //Device version
#define DEVICEV2_01_MANSTRING       "zRed"                          //Manufacturer string
#define DEVICEV2_01_PRODSTRING      "openZmeter - Capture device"   //Product string
#define DEVICEV2_01_ENDPOINT_IN       0x81                          //Endpoint de lectura de datos
#define DEVICEV2_01_ENDPOINT_OUT      0x02                          //Endpoint de escritura de datos
#define DEVICEV2_01_SAMPLEFREQ    19200.00                          //Frequencia de muestreo
#define DEVICEV2_01_TRANSFERS           32                          //Numero de transferencias a encolar
#define DEVICEV2_01_CONTROL_SETSTATUS 0x20
//----------------------------------------------------------------------------------------------------------------------
class DeviceV2_01 final : public Device {
  private :
    enum Button_Action { NONE = 0, CLICK = 1, LONG_CLICK = 2, VERY_LONG_CLICK = 3};
    enum LED_Color {OFF = 0, RED = 1, GREEN = 2, YELLOW = 3 };
    typedef struct {
      int8_t                  Voltage[3];
      int8_t                  Current[4];
      bool                    CurrentInvert[4];
      Analyzer::AnalyzerMode  Mode;
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
    bool                         pTerminate;               //Marca que deben detenerse los trabajos pendientes
    vector<Analyzer_t>           pAnalyzersSetup;          //Listado de analyzadores
    deque<Sample_t>              pBuffer;                  //Read buffer
    float                        pVoltageGain[3];          //Ganancia de tension
    float                        pCurrentGain[4];          //Ganancia de corriente
    float                        pVoltageOffset[3];        //Offset de tension
    float                        pCurrentOffset[4];        //Offset de corriente
    float                        pCurrentLevel[4];         //Current level
    bool                         pHighGain;                //Choose high gain mode
    Sample_t                     pCapture;                 //Capture status
    int                          pCaptureSync;             //Flag for detect lost samples
    struct libusb_device_handle *pDeviceHandle;            //Dispositivo de captura
    struct libusb_device        *pNewDevice;               //Dispositivo que se conecta
    libusb_context              *pContext;                 //USB context
    deque<Sample_t>              pAverageValues;           //Average wave value of last second
    uint64_t                     pLastSignalRecv;          //Last battery message recv
    bool                         pButtonAction;            //Last action readed
  private :
    static bool  GetInstances();
    static json  CheckConfig(const json &config);
    static void *Capture_Thread(void *args);
//    static void *Signal_Thread(void *args);
    static void  ReadCallback(struct libusb_transfer *transfer);
    static int   PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
  private :
    DeviceV2_01(string serial);
    bool StartDevice() override;
    bool StopDevice() override;
    void GetDevice(HTTPRequest &req) override;
    void SetDevice(HTTPRequest &req) override;
    bool BuffersAppend();
  public :
    ~DeviceV2_01();
};
//----------------------------------------------------------------------------------------------------------------------
