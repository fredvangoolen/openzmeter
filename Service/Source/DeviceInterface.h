/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Analyzer.h"
#include "FFT.h"
//----------------------------------------------------------------------------------------------------------------------
#define DEVICEINTERFACE_FILTERSTAGES  5
//----------------------------------------------------------------------------------------------------------------------
class DeviceInterface {
  public :
    typedef struct {
      float Z1;
      float Z2;
    } FilterStage_t;
    typedef struct {
      FilterStage_t *Stages;
    } FilterChannel_t;
    typedef struct {
      float            A0;
      float            A1;
      float            A2;
      float            B1;
      float            B2;
      float           *Results;
      FilterChannel_t *Channels;
    } Filter_t;
  protected :
    float     **pVoltageSamples;             //Muestras de tension
    float     **pCurrentSamples;             //Muestras de corriente
    FFT       **pVoltageFFT;                 //FFTs de tension
    FFT       **pCurrentFFT;                 //FFTs de tension
    uint8_t     pVoltageChannels;            //Canales de tension
    uint8_t     pCurrentChannels;            //Canales de corriente
    uint32_t    pSamples;                    //Numero de muestras
    uint32_t    pMaxSamples;                 //Numero maximo de muestras
    float       pSamplerate;                 //Velocidad de muestreo
    bool        pTerminate;                  //Marca que deben detenerse los trabajos pendientes
    Filter_t    pFilters;                    //Filtros aplicados para deteccion de cruce por 0
  protected :
    bool   BuffersAlloc(uint8_t voltageChannels, uint8_t currentChannels);
    void   BuffersClear();
    float *BuffersAppend(float *voltageSamples, float *CurrentSamples);
    void   CalcFFTs();

    
    
    //    typedef struct {
//      uint32_t Start;                         //Muestra dentro del bloque que inicia el ciclo
//      uint32_t End;                           //Muestra dentro del bloque que finaliza el ciclo
//    } Cycle_t;
//    typedef struct {
//      uint64_t    SequenceID;                 //Numero de secuencia del bloque
//      uint32_t    Samples;                    //Cantidad de muestras que componen el bloque
 
//      Cycle_t     Cycles[20];                 //Informacion de los ciclos del bloque
//      uint8_t     Cycles_Len;                 //Numero de ciclos en el bloque
////      Params_t    Params;                     //Parametros calculados del bloque
////      Complex_t **VoltageFFT;                 //Transformada de los canales de tension      
////      Complex_t **CurrentFFT;                 //Transformada de los canales de corriente
//    } Work_t;
  public :
    Analyzer  **Analyzers;                  //Analizadores disponibles
    uint8_t     AnalyzersCount;             //Numero de analizadores
    bool        Active;                     //Indicador de actividad de la captura
    
  protected :
      void ApplyFilter(float *in, float *out);
  protected :
      
//    Work_t pWork;                           //Bloque te trabajo
  protected :
    DeviceInterface();
    virtual ~DeviceInterface();
    void AddAnalyzer(Analyzer *analyzer);
  public :

//    uint32_t GetMaxSamples();
};

//----------------------------------------------------------------------------------------------------------------------