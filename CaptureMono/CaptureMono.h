/* ================================== zMeter ===========================================================================
 * File:     CaptureMono.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <fftw3.h>
#include "../CaptureInterface.h"
//----------------------------------------------------------------------------------------------------------------------
#define ADQUISITION_USB_VID         0x0483                //Product ID
#define ADQUISITION_PID             0x7270                //Device ID
#define ADQUISITION_ENDPOINT_IN       0x81                //Endpoint de lectura de datos
#define ADQUISITION_SAMPLEFREQ    15625.00         //Frequencia de muestreo


//#define MAX_FRAME                     3688         //Longitud maxima de muestras de un frame (236ms)
//#define ADQUISITION_SAFESAMPLES  MAX_FRAME         //Numero de muestras "seguras" antes y despues del punto de lectura 
#define PROCESSING_FREQCYCLES          500        //Numero de ciclos usados para calcular frequencia
#define PROCESSING_CURRENTOFFSET      0.10        //Corriente minima para realizar calculos
//----------------------------------------------------------------------------------------------------------------------
class CaptureMono : public CaptureInterface {
  private :
    typedef struct {
      float Voltage;                                    //Valor de tension
      float Current;                                    //Valor de corriente
    } Sample_t;
    float                        VoltageGain;             //Ganancia de tension
    float                        CurrentGain;             //Ganancia de corriente
    float                        VoltageOffset;           //Offset de tension
    float                        CurrentOffset;           //Offset de corriente
    Sample_t                    *Buffer;                  //Donde almaceno muestras leidas (RingBuffer)
    uint32_t                     ReadPos;                 //Posicion de lectura del buffer
    uint32_t                     WritePos;                //Posicion de escritura del buffer
    uint32_t                     Size;                    //Tamaño total del buffer
    uint32_t                     Used;                    //Muestras en uso del buffer
    pthread_mutex_t              CaptureMutex;            //Mutex para bloquear acceso al buffer
    pthread_cond_t               WriteWait;               //Variable de condicion para esperar si buffer esta lleno
    pthread_cond_t               ReadWait;                //Variable de condicion para esperar si buffer esta vacio
    pthread_t                    CaptureThread;           //Descriptor del hilo de captura
    bool                         Terminate;               //Señal para detener el hilo de captura
    Filter_t          Filters[5];              //Filtros paso banda para deteccion de paso por cero
    uint64_t                     SequenceID;              //Numero de secuencia de bloques generados
    struct libusb_device_handle *DeviceHandle;            //Dispositivo de captura
    
    uint32_t                     LastCycles[PROCESSING_FREQCYCLES];
    uint32_t                     LastCyclesCount;
    uint32_t                     LastCyclesPos;
    uint32_t                     LastCyclesLenght;
    pthread_mutex_t              AnalyzeMutex;                 //Mutex para bloquear acceso a muestras y resultados
    pthread_t                    AnalyzeThread;                //Descriptor del hilo de analisis
    uint64_t                     PrevTimestamp;
    uint64_t                     Agreg3S_Start;


    
  private : 
    static void  *Capture_Thread(void *args);
    static void  *Analize_Thread(void *args);
    static void   ReadCallback(struct libusb_transfer *transfer);

   
    float  CalculateFrequency(Work_t* work);
    
  private :
//    Work_t   *GetWork();
//    Params_t *Analyze(Work_t *work);
    void   SamplesCopy(float *dest, int32_t offset, uint32_t len);
  public :
    CaptureMono(char *device, uint32_t bufferLen, float baseFreq);
    bool IsRunning();
    ~CaptureMono();
};
//----------------------------------------------------------------------------------------------------------------------