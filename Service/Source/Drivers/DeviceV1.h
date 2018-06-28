/* ================================== zMeter ===========================================================================
 * File:     CaptureMono.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <libusb-1.0/libusb.h>
#include "../DeviceInterface.h"
#include "../Analyzer.h"
//----------------------------------------------------------------------------------------------------------------------
#define CAPTUREMONO_VID             0x0483       //Product ID
#define CAPTUREMONO_PID             0x7270       //Device ID
#define CAPTUREMONO_ENDPOINT_IN       0x81       //Endpoint de lectura de datos
//#define CAPTUREMONO_SAMPLEFREQ    15625.00       //Frequencia de muestreo
//#define CAPTUREMONO_BUFFERSIZE    156250         //Tamaño del buffer (10 segundos)
#define CAPTUREMONO_TRANSFERS          32       //Numero de transferencias a encolar

//#define MAX_FRAME                     3688         //Longitud maxima de muestras de un frame (236ms)
//#define ADQUISITION_SAFESAMPLES  MAX_FRAME         //Numero de muestras "seguras" antes y despues del punto de lectura 
//#define PROCESSING_FREQCYCLES          500        //Numero de ciclos usados para calcular frequencia
//#define PROCESSING_CURRENTOFFSET      0.10        //Corriente minima para realizar calculos
//----------------------------------------------------------------------------------------------------------------------
class DeviceV1 : public DeviceInterface {
  private :
//    typedef struct {
//      float Voltage;                                      //Valor de tension
//      float Current;                                      //Valor de corriente
//    } Sample_t;
    const char                  *pSerial;                 //Numero de serie del dispositivo de captura
    float                        pVoltageGain;            //Ganancia de tension
    float                        pCurrentGain;            //Ganancia de corriente
    float                        pVoltageOffset;          //Offset de tension
    float                        pCurrentOffset;          //Offset de corriente
//    Sample_t                    *Buffer;                  //Donde almaceno muestras leidas (RingBuffer)
//    uint32_t                     ReadPos;                 //Posicion de lectura del buffer
//    uint32_t                     WritePos;                //Posicion de escritura del buffer
//    uint32_t                     Used;                    //Muestras en uso del buffer
//    pthread_mutex_t              CaptureMutex;            //Mutex para bloquear acceso al buffer
//    pthread_mutex_t              AnalyzeMutex;            //Mutex para bloquear acceso a muestras y resultados
//    pthread_cond_t               WriteWait;               //Variable de condicion para esperar si buffer esta lleno
//    pthread_cond_t               ReadWait;                //Variable de condicion para esperar si buffer esta vacio
//    pthread_t                    CaptureThread;           //Descriptor del hilo de captura
//    pthread_t                    AnalyzeThread;           //Descriptor del hilo de analisis
//    Filter_t                     Filters[5];              //Filtros paso banda para deteccion de paso por cero
    struct libusb_device_handle *pDeviceHandle;           //Dispositivo de captura
    struct libusb_device        *pNewDevice;              //Dispositivo que se conecta
  private : 
    static void  *Capture_Thread(void *args);
//    static void  *Analize_Thread(void *args);
    static void   ReadCallback(struct libusb_transfer *transfer);
    static int    PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
  private :
    bool     ReadJSONFloatArray(json_object *object, float *dst, uint8_t len);
//    float    CalculateFrequency(Work_t* work);
//    uint32_t SamplesCopy(float **dest, uint32_t offset, uint32_t len);
  public :
    DeviceV1(json_object* config);
    ~DeviceV1();
};
//----------------------------------------------------------------------------------------------------------------------