/* ================================== zMeter ===========================================================================
 * File:     DeviceV2.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <libusb-1.0/libusb.h>
#include <deque>
#include "Devices.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
#define DEVICEV2_01_VID              0x0483                          //Product ID
#define DEVICEV2_01_PID              0x7270                          //Device ID
#define DEVICEV2_01_VERSION         0x0201                          //Device version
#define DEVICEV2_01_MANSTRING       "zRed"                          //Manufacturer string
#define DEVICEV2_01_PRODSTRING      "openZmeter - Capture device"   //Product string
#define DEVICEV2_01_ENDPOINT_IN       0x81                          //Endpoint de lectura de datos
#define DEVICEV2_01_SAMPLEFREQ    19200.00                          //Frequencia de muestreo
#define DEVICEV2_01_BUFFERSECONDS       10                          //Tamaño del buffer (10 segundos)
#define DEVICEV2_01_TRANSFERS           32                          //Numero de transferencias a encolar
//----------------------------------------------------------------------------------------------------------------------
class DeviceV2_01 : public Device {
  private :
    typedef struct {
      uint8_t                          OffsetV;
      uint8_t                          OffsetC;
      AnalyzerInterface::AnalyzerMode  Mode;
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
//    enum USBState {USB_RESET, USB_WAIT, USB_SYNC};
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
    
//    uint64_t                     pLastUpdate;              //Timestamp de la ultima vez que se actualizo el nombre del thread
//    char                        *pSerial;                  //Numero de serie del dispositivo de captura
//    float                        pReadSamples[7];          //Muestras leidas, previas a clasificacion
//    uint8_t                      pChannel;                 //Canal actual de lectura
//    uint8_t                      pChannelMask;             //Mascara de canales a capturar
//    uint8_t                      pChannelCount;            //Numero de canales de lectura
//    uint8_t                      pVoltageChannelCount;     //Numero de canales de tension
//    uint8_t                      pCurrentChannelCount;     //Numero de canales de corriente
//    uint32_t                     pADCSamplerate;           //Samplerate del ADC
//    uint8_t                      pOversample;              //Oversample usado en el micro
    struct libusb_device_handle *pDeviceHandle;            //Dispositivo de captura
    struct libusb_device        *pNewDevice;               //Dispositivo que se conecta
    libusb_context              *pContext;                 //USB context
        pthread_mutex_t              pMutex;
  private :
    static bool             Init();
    static void            *Capture_Thread(void *args);
    static void             ReadCallback(struct libusb_transfer *transfer);
    static int              PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
  private :
    bool               SetConfig(json config);
    AnalyzerInterface *FindAnalyzer(string serial);
    bool               Start();
    bool               Stop();
    bool               BuffersAppend();
  public :
    static void GetInstances(vector<Device*> &instances);
  public :
    uint32_t           AnalyzersCount();
    DeviceV2_01(string serial);
    ~DeviceV2_01();
};
//----------------------------------------------------------------------------------------------------------------------
