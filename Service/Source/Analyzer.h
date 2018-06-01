/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <pthread.h>
#include <inttypes.h>
#include <fftw3.h>
#include <json-c/json.h>
#include "Database.h"
#include "FFT.h"
//----------------------------------------------------------------------------------------------------------------------
#define ANALYZER_FREQCYCLES          500        //Numero de ciclos usados para calcular frequencia
//----------------------------------------------------------------------------------------------------------------------
class Analyzer : private Database {
  public :
    enum AnalyzerType {PHASE_1, PHASE_3, PHASE_3N};
    enum AggregType { AGGREG_10C, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_1H };
  private :
    typedef struct {
      uint32_t Start;                       //Muestra dentro del bloque que inicia el ciclo
      uint32_t End;                         //Muestra dentro del bloque que finaliza el ciclo
    } Cycle_t;
    typedef struct {
      AggregType  Type;                       //Tipo de agregacion
      uint64_t    Timestamp;                  //Timestamp de la muestra inicial
      uint32_t    Samples;                    //Muestras que han formado esta agregacion
      float      *RMS_V;                      //Valor RMS de la tension durante el bloque
      float      *RMS_I;                      //Valor RMS de la intensidad durante el bloque
      float      *Active_Power;               //Potencia activa (P)
      float      *Apparent_Power;             //Potencia aparente (S)
      float      *Power_Factor;               //Factor de potencia (cos(PHI))
      float      *Fundamental_Active_Power;   //Potencia activa del primer armonico (P_1)
      float      *Fundamental_Apparent_Power; //Potencia aparente fundamental (S_1)
      float      *Fundamental_Reactive_Power; //Potencia reactiva fundamental (Q_1)
      float      *Fundamental_Reactive_Energy;//Energia reactiva
      float      *Fundamental_Power_Factor;   //Factor de potencia fundamental
      float      *Active_Energy;              //Energia consumida
      float       Frequency;                  //Frecuencia
      bool        Flag;                       //Periodo marcado
      float      *THD_V;                      //Distorsion harmonica de la tension
      float      *THD_I;                      //Distorsion harmonica de la corriente
      float     **VoltageHarmonics;           //Armonicos del canal de tension
      float     **CurrentHarmonics;           //Armonicos del canal de corriente
      float      *Phi;                        //Angulo de Phi
    } Params_t;
//    typedef struct {
//      uint64_t    SequenceID;                 //Numero de secuencia del bloque
//      uint32_t    Samples;                    //Cantidad de muestras que componen el bloque
//      float     **VoltageSamples;             //Muestras de tension   (El array almacena hasta 236mS)
//      float     **CurrentSamples;             //Muestras de corriente (El array almacena hasta 236mS) 
//      Cycle_t     Cycles[20];                 //Informacion de los ciclos del bloque
//      uint8_t     Cycles_Len;                 //Numero de ciclos en el bloque
//      Params_t    Params;                     //Parametros calculados del bloque
////      Complex_t **VoltageFFT;                 //Transformada de los canales de tension      
////      Complex_t **CurrentFFT;                 //Transformada de los canales de corriente
//    } Work_t;
  private :
    uint32_t        pID;                      //ID donde el analizador guardara los datos
    char           *pName;                    //Nombre del analizador
    AnalyzerType    pType;                    //Tipo de analizador
    float           pSamplerate;              //Velocidad de muestreo
    float           pMinCurrent;              //Corriente minima
    uint32_t        pSamples;                 //Muestras analizadas
    uint64_t        pSequence;                //Numero de bloques analizados
    uint64_t        pPrevTimestamp;           //Timestamp del ultimo bloque procesado
    float         **pVoltageSamples;          //Muestras de tension del bloque
    FFT           **pVoltageFFT;              //Transformada de los canales de tension
    float         **pCurrentSamples;          //Muestras de corriente del bloque
    FFT           **pCurrentFFT;              //Transformada de los canales de corriente
    Params_t        pAggreg10C;               //Agregacion de 10 ciclos
    Params_t        pAggreg3S;                //Agregacion de 3 segundos
    Params_t        pAggreg1M;                //Agregacion de 1 minuto
    Params_t        pAggreg10M;               //Agregacion de 10 minutos
    Params_t        pAggreg1H;                //Agregacion de 1 hora
    uint32_t        pWaitingThreads;          //Hilos esperando datos
    pthread_mutex_t pWaitingThreadsMutex;     //Mutex para registrar cambios en WaitingThreads
    pthread_cond_t  pWaitingThreadsWait;      //Variable de condicion para bloquear el analizer
    bool            pDataReady;               //Datos disponibles
    pthread_mutex_t pDataReadyMutex;          //Mutex para registrar cambios en DataReady
    pthread_cond_t  pDataReadyWait;           //Variable de condicion para bloquear los clientes
    uint32_t        pPrevCycles[ANALYZER_FREQCYCLES];
    uint32_t        pPrevCyclesPos;           //Posicion de escritura de las estadisticas de ciclos
    uint32_t        pPrevCyclesLenght;        //Longitud del periodo medido
  private :
    void  AccumulateAggreg(Params_t *dst, Params_t *src);
    void  SaveAggreg(Params_t *param);
    void  AllocAggreg(Params_t *params, AggregType type);
    void  ResetAggreg(Params_t *params);
    void  NormalizeAggreg(Params_t *params);
    void  SaveWork();
    

  protected :
//    virtual uint32_t SamplesCopy(float **dest, uint32_t offset, uint32_t len) = 0;
  public :
    Analyzer(char *name, float samplerate, AnalyzerType mode, float minCurrent, uint16_t id);
    ~Analyzer();
    void         Analyze(float **voltageSamples, FFT **voltageFFT, float **currentSamples, FFT **currentFFT, uint32_t *cyclesLen, uint8_t cycles, uint64_t timestamp);
    char        *GetTypeString();
    uint32_t     GetID();
    char        *GetName();
    json_object *GetJSONCopy(char *series);

    //~Analyzer();
};
