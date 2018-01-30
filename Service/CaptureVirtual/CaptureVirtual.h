/* ================================== zMeter ===========================================================================
 * File:     CaptureVirtual.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "../CaptureInterface.h"
//----------------------------------------------------------------------------------------------------------------------
#define CAPTUREVIRTUAL_SAMPLEFREQ     15625.00        //Frequencia de muestreo
#define CAPTUREVIRTUAL_BUFFERSIZE    1562500           //Tamaño del buffer (100 segundos)
#define CAPTUREVIRTUAL_SAMPLESREAD       64           //Numero de muestras generado cada vez
#define PROCESSING_FREQCYCLES           500           //Numero de ciclos usados para calcular frequencia
#define PROCESSING_CURRENTOFFSET          0.10        //Corriente minima para realizar calculos
//----------------------------------------------------------------------------------------------------------------------
class CaptureVirtual : public CaptureInterface {
  private :
    typedef struct {
      float VoltageA;                                    //Valor de tension
      float VoltageB;                                    //Valor de tension
      float VoltageC;                                    //Valor de tension
      float CurrentA;                                    //Valor de corriente
      float CurrentB;                                    //Valor de corriente
      float CurrentC;                                    //Valor de corriente
      float CurrentD;                                    //Valor de corriente
    } Sample_t;
    Sample_t                    *Buffer;                  //Donde almaceno muestras leidas (RingBuffer)
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
  private : 
    static void  *Capture_Thread(void *args);
    static void  *Analize_Thread(void *args);
    static void   ReadCallback(struct libusb_transfer *transfer);
    static int    PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
  private :
    float    CalculateFrequency(Work_t* work);
    uint32_t SamplesCopy(float **dest, uint32_t offset, uint32_t len);
  public :
    CaptureVirtual(uint32_t samplerate, bool triphase, float nominalcurrent, uint16_t id);
    ~CaptureVirtual();
};
//----------------------------------------------------------------------------------------------------------------------