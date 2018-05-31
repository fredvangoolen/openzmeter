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
#define PROCESSING_FREQCYCLES          500        //Numero de ciclos usados para calcular frequencia
//----------------------------------------------------------------------------------------------------------------------
class Analyzer : private Database {
  public :
    enum AnalyzerType {FASE_1, FASE_3, FASE_3N};
    enum AggregType { AGGREG_10C, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_1H };
    typedef struct {
      uint32_t Start;                       //Muestra dentro del bloque que inicia el ciclo
      uint32_t End;                         //Muestra dentro del bloque que finaliza el ciclo
    } Cycle_t;
    typedef struct {
      uint64_t    Timestamp;                  //Timestamp de la muestra inicial
      AggregType  Type;                       //Tipo de agregacion
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
    typedef struct {
      uint16_t    *Frequency;                 //Distribucion de frecuencias
      uint16_t    *Voltage;                   //Distribucion de tensiones
      uint16_t    *THD;                       //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
      uint16_t    Harmonics[2];               //Numero de muestras que cumplen los limites de harmonicos / no cumplen
    } Stats_t;
    typedef struct {
      uint64_t    SequenceID;                 //Numero de secuencia del bloque
      uint32_t    Samples;                    //Cantidad de muestras que componen el bloque
      float     **VoltageSamples;             //Muestras de tension   (El array almacena hasta 236mS)
      float     **CurrentSamples;             //Muestras de corriente (El array almacena hasta 236mS) 
      Cycle_t     Cycles[20];                 //Informacion de los ciclos del bloque
      uint8_t     Cycles_Len;                 //Numero de ciclos en el bloque
      Params_t    Params;                     //Parametros calculados del bloque
//      Complex_t **VoltageFFT;                 //Transformada de los canales de tension      
//      Complex_t **CurrentFFT;                 //Transformada de los canales de corriente
    } Work_t;
  private :

  private :
    uint32_t        pWaitingThreads;       //Hilos esperando datos
    pthread_mutex_t pWaitingThreadsMutex;  //Mutex para registrar cambios en WaitingThreads
    pthread_cond_t  pWaitingThreadsWait;   //Variable de condicion para bloquear el analizer
    bool            pDataReady;            //Datos disponibles
    pthread_mutex_t pDataReadyMutex;       //Mutex para registrar cambios en DataReady
    pthread_cond_t  pDataReadyWait;        //Variable de condicion para bloquear los clientes
    uint32_t        pLastCycles[PROCESSING_FREQCYCLES];
    uint32_t        pLastCyclesPos;
    uint32_t        pLastCyclesLenght;
    uint32_t        pMaxSamples;

    Params_t        pAggreg3S;
    Params_t        pAggreg1M;
    Params_t        pAggreg10M;
    Params_t        pAggreg1H;
    uint16_t        pNodeIndex;
    Stats_t         pStats;

  private :
    void  SaveParams(Params_t *param);
    

  public :
    uint32_t  ID;
    char     *Name;
    
    
    
    
    uint64_t    pPrevTimestamp;
    Work_t      pWork;
    bool        pTerminate;
        bool            pTriphase;
            float           pSamplerate;
  public :
    void     AllocParams(Params_t *params, AggregType type);
    void     ResetParams(Params_t *params);
    void     NormalizeParams(Params_t *params);
    void     ResetWork();
    void     ResetStats();
    void     SaveWorkParams();
    void     Analyze(float minimunCurrent);

    void     AccumulateAggregation(Params_t *dst, Params_t *src);

  protected :


//    static float  CalcPhi(Complex_t voltage, Complex_t current);
  protected :
//    virtual uint32_t SamplesCopy(float **dest, uint32_t offset, uint32_t len) = 0;
  public :
    bool         Triphase();
    uint32_t     SampleRate();
    json_object *GetJSONCopy(char *series);
  public :
    Analyzer(char *name, float samplerate, uint32_t maxSamples, bool triphase, uint16_t id);
    //~Analyzer();
};
