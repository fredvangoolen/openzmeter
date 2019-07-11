/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <pthread.h>
#include <inttypes.h>
#include <deque>
#include "Analyzers.h"
#include "FFT.h"
#include "Events.h"
//----------------------------------------------------------------------------------------------------------------------
#define ZEROCROSSING_FILTERSTAGES          5
#define ZEROCROSSING_SECONDS              10        //Numero de segundos usados para calcular frequencia
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft : public AnalyzerInterface {
  private :
    typedef struct {
      char  *Buffer;
      size_t Size;
      size_t MaxSize;
    } CurlBuff_t;
    typedef struct {
      float                 A0;
      float                 A1;
      float                 A2;
      float                 B1;
      float                 B2;
      vector<float>         ChannelsZ1;
      vector<float>         ChannelsZ2;
    } Filter_t;
  private :
    uint64_t                   pSequence;                //Numero de bloques analizados
    Params_t                   pAggreg10C;               //Agregacion de 10 ciclos
    Params_t                   pAggreg3S;                //Agregacion de 3 segundos
    Params_t                   pAggreg1M;                //Agregacion de 1 minuto
    Params_t                   pAggreg10M;               //Agregacion de 10 minutos
    Params_t                   pAggreg15M;               //Agregacion de 15 minutos
    Params_t                   pAggreg1H;                //Agregacion de 1 hora
    Events                    *pEvents;                  //Events detector
    vector<FFT*>               pVoltageFFT;              //Muestras de FFT tension del bloque
    vector<FFT*>               pCurrentFFT;              //Muestras de FFT corriente del bloque
    bool                       pWorking;                 //Flag para indicar que el hilo esta trabajando
    pthread_t                  pThread;                  //Hilo de ejecucion
    pthread_mutex_t            pThreadMutex;             //Mutex para la variable de condicion
    pthread_cond_t             pThreadCond;              //Variable de condicion para esperar
    bool                       pTerminate;               //Marca para finalizar proceso
    uint64_t                   pPrevTimestamp;           //Timestamp del ultimo bloque procesado
    uint32_t                   pWaitingThreads;          //Hilos esperando datos
    pthread_mutex_t            pWaitingThreadsMutex;     //Mutex para registrar cambios en WaitingThreads
    pthread_cond_t             pWaitingThreadsCond;      //Variable de condicion para bloquear el analizer
    bool                       pDataReady;               //Datos disponibles
    pthread_mutex_t            pDataReadyMutex;          //Mutex para registrar cambios en DataReady
    pthread_cond_t             pDataReadyCond;           //Variable de condicion para bloquear los clientes
    deque<vector<float> >      pDelayVoltage;            //Delay buffer for samples
    deque<vector<float> >      pDelayCurrent;            //Delay buffer for samples
    vector<Filter_t>           pFilters;                 //Filter bank
    deque<uint32_t>            pCycles;                  //Prev cycles lenghts
    vector<uint32_t>           pBlockCycles;             //Current block cycles lengths
    vector<float>              pLastSamples;             //Filter samples backup
    uint32_t                   pCycleLen;                //Current cycle len
    uint32_t                   pMaxSamples;              //Max block samples
  private :
    static void  *Analyzer_Thread(void *args);
  private :
    float PercentChange(float in, float reference);
    float CalcFrequency();
  public :
    AnalyzerSoft(string id, AnalyzerInterface::AnalyzerMode mode, float samplerate);
    ~AnalyzerSoft();
    void GetParams(Params_t &params, ParamsType &copy);
    bool BufferAppend(const vector<float> &voltageSamples, const vector<float> &currentSamples, uint64_t sampletime);
    bool IsFinished();
};
