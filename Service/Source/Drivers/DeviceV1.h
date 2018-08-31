/* ================================== zMeter ===========================================================================
 * File:     CaptureMono.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <libusb-1.0/libusb.h>
#include "DeviceInterface.h"
#include "Analyzers/AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
#define DEVICEV1_VID             0x0483       //Product ID
#define DEVICEV1_PID             0x7270       //Device ID
#define DEVICEV1_ENDPOINT_IN       0x81       //Endpoint de lectura de datos
#define DEVICEV1_SAMPLEFREQ    15625.00       //Frequencia de muestreo
#define DEVICEV1_BUFFERSECONDS       10       //Tamaño del buffer (10 segundos)
//----------------------------------------------------------------------------------------------------------------------


//#define CAPTUREMONO_BUFFERSIZE    156250         //Tamaño del buffer (10 segundos)
#define DEVICEV1_TRANSFERS          32       //Numero de transferencias a encolar

//#define MAX_FRAME                     3688         //Longitud maxima de muestras de un frame (236ms)
//#define ADQUISITION_SAFESAMPLES  MAX_FRAME         //Numero de muestras "seguras" antes y despues del punto de lectura 
//#define PROCESSING_FREQCYCLES          500        //Numero de ciclos usados para calcular frequencia
//#define PROCESSING_CURRENTOFFSET      0.10        //Corriente minima para realizar calculos
//----------------------------------------------------------------------------------------------------------------------
class DeviceV1 : public DeviceInterface {
  private :
    AnalyzerSoft                *pAnalyzer;                //Analizador
    uint32_t                     pUsed;                    //Muestras en uso del buffer
    uint32_t                     pBuffLen;                 //Tamaño del buffer
    float                       *pVoltageSamples;          //Muestras de tension
    float                       *pCurrentSamples;          //Muestras de corriente
    uint32_t                     pReadPos;                 //Posicion de lectura del buffer
    uint32_t                     pWritePos;                //Posicion de escritura del buffer
    uint64_t                     pLastUpdate;              //Timestamp de la ultima vez que se actualizo el nombre del thread
    char                        *pSerial;                  //Numero de serie del dispositivo de captura
    float                        pVoltageGain;             //Ganancia de tension
    float                        pCurrentGain;             //Ganancia de corriente
    float                        pVoltageOffset;           //Offset de tension
    float                        pCurrentOffset;           //Offset de corriente
    struct libusb_device_handle *pDeviceHandle;            //Dispositivo de captura
    struct libusb_device        *pNewDevice;               //Dispositivo que se conecta    
  private : 
    static void  *Capture_Thread(void *args);
    static void   ReadCallback(struct libusb_transfer *transfer);
    static int    PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
  private :
    bool BuffersAppend(float voltageSample, float currentSample);
    bool ReadJSONFloatArray(json_object *object, float *dst, uint8_t len);
    AnalyzerInterface *FindAnalyzer(char *serial);
  public :
    DeviceV1(json_object* config);
    ~DeviceV1();
};
//----------------------------------------------------------------------------------------------------------------------