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
class AnalyzerSoft : public AnalyzerInterface {
  private :
    uint8_t             pVoltageChannels;         //Canales de tension
    uint8_t             pCurrentChannels;         //Canales de corriente


    
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
//    void  Init(char *name, char *id, float samplerate, float minCurrent);
//    void  AccumulateAggreg(Params_t *dst, Params_t *src);
//    void  SaveAggreg(Params_t *param);
//    void  ResetAggreg(Params_t *params);
//    void  NormalizeAggreg(Params_t *params);
    void  Analyze();
  protected :
//    virtual uint32_t SamplesCopy(float **dest, uint32_t offset, uint32_t len) = 0;
    void GetParams(Params_t *params, ExtParams_t *extParams);
    void ReleaseParams();
  public :
    AnalyzerSoft(float samplerate, char *id, float nominal_voltage, float nominal_frequency, char *name, float max_current, float min_current, AnalyzerInterface::AnalyzerMode mode);
    ~AnalyzerSoft();
    bool         BufferAppend(float *voltageSamples, float *CurrentSamples, uint64_t sampletime);
    bool         IsFinished();
//    char        *GetTypeString();
//    char        *GetID();
//    char        *GetName();

};
