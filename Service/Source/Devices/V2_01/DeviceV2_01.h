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
      uint8_t Index;
      float   Invert;
    } CurrentCH_t;
    typedef struct {
      vector<uint8_t>         Voltage;
      vector<CurrentCH_t>     Current;
      Analyzer::AnalyzerMode  Mode;
      AnalyzerSoft           *Parent;
    } Analyzer_t;
    typedef struct {
      float Voltage[3];
      float Current[4];
    } Sample_t;
    class Transfer {
      private :
        libusb_transfer *pTransfer;
        uint8_t         *pBuffer;
        uint32_t         pBufferSize;
        DeviceV2_01     *pParent;
      public :
        Transfer(DeviceV2_01 *parent, uint32_t size);
        ~Transfer();
        int      Submit();
        int      Submit(void *data, uint32_t size);
        int      Submit(libusb_device_handle *deviceHandle, uint8_t endpoint, libusb_transfer_cb_fn callback, int32_t timeout);
        void     Setup(libusb_device_handle *deviceHandle, uint8_t endpoint, libusb_transfer_cb_fn callback, int32_t timeout);
        uint32_t BufferSize();
    };   
  private :
    static bool                  pRegistered;    
  private :
    uint8_t                      pSubmittedTransfers;      //Keep count of currently submitted transfers
    pthread_t                    pCaptureThread;           //Descriptor del hilo de captura
    bool                         pTerminate;               //Marca que deben detenerse los trabajos pendientes
    vector<Analyzer_t>           pAnalyzersSetup;          //Listado de analyzadores
    deque<Sample_t>              pBuffer;                  //Read buffer
    vector<float>                pVoltageGain;             //Ganancia de tension
    vector<float>                pCurrentGain;             //Ganancia de corriente
    vector<float>                pVoltageOffset;           //Offset de tension
    vector<float>                pCurrentOffset;           //Offset de corriente
    vector<float>                pCurrentLevel;            //Current level
    bool                         pHighGain;                //Choose high gain mode
    vector<int16_t>              pCapture;                 //Capture status
    libusb_device_handle        *pDeviceHandle;            //Dispositivo de captura
    libusb_device               *pNewDevice;               //Dispositivo que se conecta
    libusb_context              *pContext;                 //USB context
    deque<Sample_t>              pAverageValues;           //Average wave value of last second
    RWLocks                      pAverageLocks;            //Lock access to average values
    uint64_t                     pLastSignalRecv;          //Last battery message recv
    bool                         pButtonAction;            //Last action readed
    vector<Transfer>             pTransfers;
    Transfer                     pSignalTransfer;
  private :
    static bool  GetInstances();
    static json  CheckConfig(const json &config);
    static void *Capture_Thread(void *args);
    static void  ReadCallback(struct libusb_transfer *transfer);
    static int   PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
  private :
    DeviceV2_01(const string &serial);
    bool StartDevice() override;
    bool StopDevice() override;
    void GetDevice(HTTPRequest &req) override;
    void SetDevice(HTTPRequest &req) override;
    bool BuffersAppend();
  public :
    ~DeviceV2_01();
};
//----------------------------------------------------------------------------------------------------------------------
