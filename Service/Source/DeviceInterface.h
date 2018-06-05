/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Analyzer.h"
#include "FFT.h"
//----------------------------------------------------------------------------------------------------------------------
#define DEVICEINTERFACE_BUFFERSECONDS     10        //Tamaño del buffer (10 segundos)
//----------------------------------------------------------------------------------------------------------------------
class DeviceInterface {
  private :
    char            *pDriverName;              //Nombre del driver (para mostrar en el thread)
    uint8_t          pVoltageChannels;         //Canales de tension
    uint8_t          pCurrentChannels;         //Canales de corriente
    float           *pVoltageSamples;          //Muestras de tension
    float           *pCurrentSamples;          //Muestras de corriente
    uint32_t         pReadPos;                 //Posicion de lectura del buffer
    uint32_t         pWritePos;                //Posicion de escritura del buffer
    uint32_t         pUsed;                    //Muestras en uso del buffer
    uint32_t         pBuffLen;                 //Tamaño del buffer
    float            pSamplerate;              //Velocidad de muestreo
    Analyzer       **pAnalyzers;               //Analizadores disponibles
    uint8_t          pAnalyzersCount;          //Numero de analizadores
    float            pMaxCurrent;              //Corriente maxima de los sensores
    float            pMinCurrent;              //Corriente minima de los sensores
    uint64_t         pLastUpdate;              //Timestamp de la ultima vez que se actualizo el nombre del thread
  protected :
    pthread_t        pCaptureThread;           //Descriptor del hilo de captura
    bool             pActive;                  //Indicador de actividad de la captura
    bool             pTerminate;               //Marca que deben detenerse los trabajos pendientes  
  protected :
    bool   BuffersAlloc(uint8_t voltageChannels, uint8_t currentChannels, float samplerate);
    void   BuffersReset();
    bool   BuffersAppend(float *voltageSamples, float *CurrentSamples);
    void   AddAnalyzer(Analyzer *analyzer);
    DeviceInterface(char *driver);
  public :
    virtual  ~DeviceInterface();
    bool      IsActive();
    float     GetSamplerate();
    float     GetMaxCurrent();
    uint8_t   GetAnalyzers();
    Analyzer *GetAnalyzers(uint8_t idx);
};

//----------------------------------------------------------------------------------------------------------------------