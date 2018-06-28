/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <pthread.h>
#include <inttypes.h>
#include <json-c/json.h>
#include "AnalyzerInterface.h"
#include "FFT.h"
#include "ZeroCrossing.h"
#include "Stats.h"
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft : private AnalyzerInterface {
  public :
    enum AnalyzerType {PHASE_1, PHASE_3, PHASE_3N};
  private :
    enum AggregType { AGGREG_10C, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_1H };
    typedef struct {
      AggregType Type;                          //Tipo de agregacion
      uint64_t   Timestamp;                     //Timestamp de la muestra inicial
      uint32_t   Samples;                       //Muestras que han formado esta agregacion
      float      RMS_V[3];                      //Valor RMS de la tension durante el bloque
      float      RMS_I[4];                      //Valor RMS de la intensidad durante el bloque
      float      Active_Power[3];               //Potencia activa (P)
      float      Apparent_Power[3];             //Potencia aparente (S)
      float      Power_Factor[3];               //Factor de potencia (cos(PHI))
      float      Fundamental_Active_Power[3];   //Potencia activa del primer armonico (P_1)
      float      Fundamental_Apparent_Power[3]; //Potencia aparente fundamental (S_1)
      float      Fundamental_Reactive_Power[3]; //Potencia reactiva fundamental (Q_1)
      float      Fundamental_Reactive_Energy[3];//Energia reactiva
      float      Fundamental_Power_Factor[3];   //Factor de potencia fundamental
      float      Active_Energy[3];              //Energia consumida
      float      Frequency;                     //Frecuencia
      bool       Flag;                          //Periodo marcado
      float      THD_V[3];                      //Distorsion harmonica de la tension
      float      THD_I[4];                      //Distorsion harmonica de la corriente
      float      VoltageHarmonics[3][50];       //Armonicos del canal de tension
      float      CurrentHarmonics[4][50];       //Armonicos del canal de corriente
      float      Phi[3];                        //Angulo de Phi
    } Params_t;
  private :
    char           *pID;                      //ID donde el analizador guardara los datos
    char           *pName;                    //Nombre del analizador
    AnalyzerType    pType;                    //Tipo de analizador
    float           pSamplerate;              //Velocidad de muestreo
    float           pMinCurrent;              //Corriente minima
    uint32_t        pSamples;                 //Muestras analizadas
    uint64_t        pSequence;                //Numero de bloques analizados
    Params_t        pAggreg10C;               //Agregacion de 10 ciclos
    Params_t        pAggreg3S;                //Agregacion de 3 segundos
    Params_t        pAggreg1M;                //Agregacion de 1 minuto
    Params_t        pAggreg10M;               //Agregacion de 10 minutos
    Params_t        pAggreg1H;                //Agregacion de 1 hora
    ZeroCrossing   *pZC;                      //Generador de paso por cero y calculo de frecuencia
    Stats          *pStats;                   //Registro de estadisticas
    uint32_t        pMaxSamples;              //Numero maximo de muestras
    float          *pVoltageSamples[3];       //Muestras de tension del bloque
    float          *pCurrentSamples[4];       //Muestras de corriente del bloque
    FFT            *pVoltageFFT[3];           //Muestras de FFT tension del bloque
    FFT            *pCurrentFFT[4];           //Muestras de FFT corriente del bloque
    uint32_t        pCyclesLen[20];           //Longitud de los ciclos detectados
    uint8_t         pCycles;                  //Numero de ciclos detectados
    bool            pWorking;                 //Flag para indicar que el hilo esta trabajando
    pthread_t       pThread;               //Hilo de ejecucion
    pthread_mutex_t pThreadMutex;          //Mutex para la variable de condicion
    pthread_cond_t  pThreadCond;           //Variable de condicion para esperar
    bool            pTerminate;            //Marca para finalizar proceso
    uint64_t        pPrevTimestamp;           //Timestamp del ultimo bloque procesado
    uint32_t        pWaitingThreads;          //Hilos esperando datos
    pthread_mutex_t pWaitingThreadsMutex;     //Mutex para registrar cambios en WaitingThreads
    pthread_cond_t  pWaitingThreadsCond;      //Variable de condicion para bloquear el analizer
    bool            pDataReady;               //Datos disponibles
    pthread_mutex_t pDataReadyMutex;          //Mutex para registrar cambios en DataReady
    pthread_cond_t  pDataReadyCond;           //Variable de condicion para bloquear los clientes
  private :
    static void *Analyzer_Thread(void *args);
  private :
    void  Init(char *name, char *id, float samplerate, float minCurrent);
    void  AccumulateAggreg(Params_t *dst, Params_t *src);
    void  SaveAggreg(Params_t *param);
    void  ResetAggreg(Params_t *params);
    void  NormalizeAggreg(Params_t *params);
    void  Analyze();
  protected :
//    virtual uint32_t SamplesCopy(float **dest, uint32_t offset, uint32_t len) = 0;
  public :
    AnalyzerSoft(char *name, bool useNeutral, char *id, float samplerate, float minCurrent);
    AnalyzerSoft(char *name, char *id, float samplerate, float minCurrent);
    ~AnalyzerSoft();
    bool         SampleAppend(float *voltageSamples, float *CurrentSamples, uint64_t sampletime);
    bool         IsFinished();
    char        *GetTypeString();
    char        *GetID();
    char        *GetName();
    json_object *GetJSONCopy(char *series);
};
