/* ================================== zMeter ===========================================================================
 * File:     CaptureV1.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <libusb-1.0/libusb.h>
#include "DeviceInterface.h"
#include "AnalyzerSoft.h"
#include "RingBuffer.h"
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
//----------------------------------------------------------------------------------------------------------------------
class DeviceV1_00 : public DeviceInterface {
  private :
    typedef struct {
      float Voltage;
      float Current;
    } Sample_t;
  private :
    AnalyzerSoft                *pAnalyzer;                //Analizador
    RingBuffer<Sample_t>         pBuffer;                  //Read buffer
    uint64_t                     pLastUpdate;              //Timestamp de la ultima vez que se actualizo el nombre del thread
    char                        *pSerial;                  //Numero de serie del dispositivo de captura
    float                        pVoltageGain;             //Ganancia de tension
    float                        pCurrentGain;             //Ganancia de corriente
    float                        pVoltageOffset;           //Offset de tension
    float                        pCurrentOffset;           //Offset de corriente
    struct libusb_device_handle *pDeviceHandle;            //Dispositivo de captura
    struct libusb_device        *pNewDevice;               //Dispositivo que se conecta    
    pthread_t                    pSignalThread;            //Thread to handle LED and sounds
    pthread_mutex_t              pMutex;                   //Start/Stop mutex
    json_object                 *pConfig;                  //Configuration section
    libusb_context              *pContext;                 //USB context
  private :
    static bool                  pClassRegistered;
  private :
    static bool             Init();
    static void            *Capture_Thread(void *args);
    static void            *Signal_Thread(void *args);
    static void             ReadCallback(struct libusb_transfer *transfer);
    static int              PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
  private :
    bool               ConfigDev(json_object *config);
    AnalyzerInterface *FindAnalyzer(const char *serial);
    uint32_t           AnalyzersCount();
    const char        *Serial();
    const char        *Driver();
    const char        *Name();
    bool               Start();
    bool               Stop();
    bool               BuffersAppend(float voltageSample, float currentSample);
  public :
    ~DeviceV1_00();
};
//----------------------------------------------------------------------------------------------------------------------
