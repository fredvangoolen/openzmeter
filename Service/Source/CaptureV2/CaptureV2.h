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
#define CAPTURE_V2_VID              0x0483        //Product ID
#define CAPTURE_V2_PID              0x7270        //Device ID
#define CAPTURE_V2_ENDPOINT_IN        0x81        //Endpoint de lectura de datos
#define CAPTURE_V2_SAMPLEFREQ     15625.00       //Frequencia de muestreo
#define CAPTURE_V2_BUFFERSIZE       156250         //Tamaño del buffer (10 segundos)
#define CAPTURE_V2_TRANSFERS            32        //Numero de transferencias a encolar
#define PROCESSING_FREQCYCLES          500        //Numero de ciclos usados para calcular frequencia
#define PROCESSING_CURRENTOFFSET      0.10        //Corriente minima para realizar calculos
//----------------------------------------------------------------------------------------------------------------------
class CaptureV2 : public DeviceInterface {
  private :    
    float                        pVoltageGain[3];          //Ganancia de tension
    float                        pCurrentGain[4];          //Ganancia de corriente
    float                        pVoltageOffset[3];        //Offset de tension
    float                        pCurrentOffset[4];        //Offset de corriente
    


    //    typedef struct {
//      float Voltage[3];                                   //Valor de tension
//      float Current[4];                                   //Valor de corriente
//    } Sample_t;
    uint8_t                      CurrentChannel;          //Canal actual de lectura
    uint8_t                      ChannelsCount;           //Numero de canales de lectura
    uint8_t                      Oversample;              //Oversample usado en el micro
    const char                  *Serial;                  //Numero de serie del dispositivo de captura
    float                       *Buffer;                  //Donde almaceno muestras leidas (RingBuffer)
    uint32_t                     ReadPos;                 //Posicion de lectura del buffer
    uint32_t                     WritePos;                //Posicion de escritura del buffer
    uint32_t                     Used;                    //Muestras en uso del buffer
    pthread_mutex_t              CaptureMutex;            //Mutex para bloquear acceso al buffer
    pthread_mutex_t              AnalyzeMutex;            //Mutex para bloquear acceso a muestras y resultados
    pthread_cond_t               WriteWait;               //Variable de condicion para esperar si buffer esta lleno
    pthread_cond_t               ReadWait;                //Variable de condicion para esperar si buffer esta vacio
    pthread_t                    CaptureThread;           //Descriptor del hilo de captura
    pthread_t                    AnalyzeThread;           //Descriptor del hilo de analisis
    Filter_t                     Filters[5];              //Filtros paso banda para deteccion de paso por cero
    struct libusb_device_handle *DeviceHandle;            //Dispositivo de captura
    struct libusb_device        *NewDevice;               //Dispositivo que se conecta
  private : 
    static void  *Capture_Thread(void *args);
    static void  *Analize_Thread(void *args);
    static void   ReadCallback(struct libusb_transfer *transfer);
    static int    PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
  private :
    float    CalculateFrequency(Analyzer::Work_t* work);
    uint32_t SamplesCopy(float **dest, uint32_t offset, uint32_t len);
    CaptureV2();
  public :
    static CaptureV2 *Init(json_object *config);
    bool Start();
    ~CaptureV2();
};
//----------------------------------------------------------------------------------------------------------------------
