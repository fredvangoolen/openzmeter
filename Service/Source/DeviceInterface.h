/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Analyzer.h"
#include "FFT.h"
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
      FilterChannel_t *Channels;
    } Filter_t;
    //      float     **VoltageSamples;             //Muestras de tension   (El array almacena hasta 236mS)
//      float     **CurrentSamples;             //Muestras de corriente (El array almacena hasta 236mS)
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
    Analyzer      **pAnalyzers;             //Analizadores disponibles
    uint32_t        pAnalyzersCount;        //Numero de analizadores
    float           pSamplerate;            //Velocidad de muestreo
    uint32_t        pID;                    //Identificador del dispositivo (Tiene que ser multiplo de 4)
    uint64_t        pSequenceID;            //Numero de secuencia del bloque analizado
  protected :
      void ApplyFilter(float *in, float *out);
  protected :
      bool pTerminate;
//    Work_t pWork;                           //Bloque te trabajo
  protected :
    DeviceInterface();
    virtual ~DeviceInterface();
  public :
    virtual bool Start() = 0;
    uint32_t GetMaxSamples();
};

//----------------------------------------------------------------------------------------------------------------------