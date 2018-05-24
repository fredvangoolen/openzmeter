/* ================================== zMeter ===========================================================================
 * File:     CaptureMono.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <libusb-1.0/libusb.h>
#include "../DeviceInterface.h"
#include "../Analyzer.h"
#include "../Tools.h"
//----------------------------------------------------------------------------------------------------------------------
#define CAPTUREV2_VID              0x0483        //Product ID
#define CAPTUREV2_PID              0x7270        //Device ID
#define CAPTUREV2_ENDPOINT_IN        0x81        //Endpoint de lectura de datos
#define CAPTUREV2_BUFFERSIZE           10         //Tamaño del buffer (10 segundos)
#define CAPTUREV2_TRANSFERS            32        //Numero de transferencias a encolar


//#define PROCESSING_FREQCYCLES          500        //Numero de ciclos usados para calcular frequencia
//#define PROCESSING_CURRENTOFFSET      0.10        //Corriente minima para realizar calculos
//----------------------------------------------------------------------------------------------------------------------
class DeviceV2 : public DeviceInterface {
  private :    
    const char                  *pSerial;                  //Numero de serie del dispositivo de captura
    float                       *pBuffer;                  //Donde almaceno muestras leidas (RingBuffer)
    float                        pVoltageGain[3];          //Ganancia de tension
    float                        pCurrentGain[4];          //Ganancia de corriente
    float                        pVoltageOffset[3];        //Offset de tension
    float                        pCurrentOffset[4];        //Offset de corriente
    uint8_t                      pChannelMask;             //Mascara de canales a capturar
    uint8_t                      pChannelCount;             //Numero de canales de lectura
    uint32_t                     pReadPos;                 //Posicion de lectura del buffer
    uint32_t                     pWritePos;                //Posicion de escritura del buffer
    uint32_t                     pUsed;                    //Muestras en uso del buffer
    uint32_t                     pBuffLen;                 //Tamaño del buffer
    struct libusb_device_handle *pDeviceHandle;            //Dispositivo de captura
    struct libusb_device        *pNewDevice;               //Dispositivo que se conecta
    pthread_mutex_t              pCaptureMutex;            //Mutex para bloquear acceso al buffer
    pthread_mutex_t              pAnalyzeMutex;            //Mutex para bloquear acceso a muestras y resultados
    pthread_cond_t               pWriteWait;               //Variable de condicion para esperar si buffer esta lleno
    pthread_cond_t               pReadWait;                //Variable de condicion para esperar si buffer esta vacio
    uint8_t                      pOversample;              //Oversample usado en el micro
    uint8_t                      pCurrentChannel;          //Canal actual de lectura


    pthread_t                    CaptureThread;           //Descriptor del hilo de captura
    pthread_t                    AnalyzeThread;           //Descriptor del hilo de analisis
    Filter_t                     Filters[5];              //Filtros paso banda para deteccion de paso por cero
  private : 
    static void  *Capture_Thread(void *args);
    static void  *Analize_Thread(void *args);
    static void   ReadCallback(struct libusb_transfer *transfer);
    static int    PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
  private :
    float    CalculateFrequency(Analyzer::Work_t* work);
    uint32_t SamplesCopy(float **dest, uint32_t offset, uint32_t len);
    DeviceV2();
  public :
    static DeviceV2 *Init(json_object *config);
    bool Start();
    ~DeviceV2();
};
//----------------------------------------------------------------------------------------------------------------------
