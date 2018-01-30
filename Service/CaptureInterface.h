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
//---------------------------------------------------------------------------------------------------------------------
#define PROCESSING_FREQCYCLES          500        //Numero de ciclos usados para calcular frequencia
#define EVENT_RVC                      5.0        //Variacion en % para generar RVC
#define EVENT_RVC_HYSTERESIS           3.0        //Variacion en % para finalizar RVC
#define EVENT_DIP_SWELL               10.0        //Variacion en % para generar subida o bajada
#define EVENT_INTERRUPTION            90.0        //Bajada en % para generar interrupcion
#define EVENT_HYSTERESIS              06.0        //Variacion en % para cerrar evento (DIP/SWELL/INTERRUPTION)
#define EVENT_ENVELOPETIME             0.1        //Tiempo en segundos que se guarda antes y despues de un evento
//---------------------------------------------------------------------------------------------------------------------
class CaptureInterface : private Database {
  public :
    enum Aggreg_Type { AGGREG_10C, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_1H };
    enum Event_Type  { EVT_NONE, EVT_SWELL, EVT_DIP, EVT_INTERRUPTION, EVT_RVC, EVT_GLITCH };
    static const float FrequencyRanges[];   //Rangos de frecuencias para estadisticas
    typedef struct {
      uint32_t Start;                       //Muestra dentro del bloque que inicia el ciclo
      uint32_t End;                         //Muestra dentro del bloque que finaliza el ciclo
    } Cycle_t;
    typedef struct {
      float Real;                           //Parte real
      float Imag;                           //Parte imaginaria
    } Complex_t;
    typedef struct {
      float     *Input;                     //Muestras de entrada
      Complex_t *Output;                    //Datos de salida
      uint32_t   Len;                       //Longitud de la transformada
      fftwf_plan Plan;                      //Plan para realizar la transformada
      pthread_t  Thread;                    //Hilo de ejecucion
    } FFT_t;
    typedef struct {
      uint64_t    Timestamp;                  //Timestamp de la muestra inicial
      Aggreg_Type Type;                       //Tipo de agregacion
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
      uint16_t    *Frequency;                 //Distribucion de frecuencias (Segun valores de arriba)
//      uint16_t    *Voltage;                   //Cada posicion representa un rango desde -10% hasta +10% (y el ultimo otros valores)
      
//      uint16_t    THD[40];                    //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
//      uint16_t    Harmonics[2];               //Numero de muestras que cumplen los limites de harmonicos / no cumplen
    } Stats_t;
    typedef struct {
      uint64_t    SequenceID;                 //Numero de secuencia del bloque
      uint32_t    Samples;                    //Cantidad de muestras que componen el bloque
      float     **VoltageSamples;             //Muestras de tension   (El array almacena hasta 236mS)
      float     **CurrentSamples;             //Muestras de corriente (El array almacena hasta 236mS) 
      Cycle_t     Cycles[20];                 //Informacion de los ciclos del bloque
      uint8_t     Cycles_Len;                 //Numero de ciclos en el bloque
      Params_t    Params;                     //Parametros calculados del bloque
      Complex_t **VoltageFFT;                 //Transformada de los canales de tension      
      Complex_t **CurrentFFT;                 //Transformada de los canales de corriente
    } Work_t;
    typedef struct {
      float A0;
      float A1;
      float A2;
      float B1;
      float B2;
      float Z1;
      float Z2;
    } Filter_t;
  private :
    typedef struct {
      float    *RMS;
      uint64_t  Timestamp;
      uint32_t  Len;
    } EventCycle_t;
    typedef struct {
      enum Event_Type Type;                 //Tipo de evento (puede cambiar durante el evento)
      uint64_t        Start;                //Timestamp de inicio
      uint64_t        Duration;             //Duracion en samples
      float           Voltage;              //Tension maxima o minima durante el evento
      float           Change;               //Porcentage de desviacion del evento
    } Event_t;
  private :
    static pthread_mutex_t pFFTPlannerMutex; //Bloquea el acceso simultaneo al plinificador de FFTs
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
    uint32_t        pSampleRate;
    uint32_t        pMaxSamples;
    bool            pTriphase;
    Params_t        pAggreg3S;
    Params_t        pAggreg1M;
    Params_t        pAggreg10M;
    Params_t        pAggreg1H;
    uint16_t        pNodeIndex;
    Stats_t         pStats;
    EventCycle_t    pPrevCycles[100];     //Informacion de ciclos anteriores para calculo de eventos
    uint8_t         pPrevCyclesLen;       //Tamaño de buffer de ciclos
    uint8_t         pPrevCyclesWrite;     //Posicion actual del buffer de ciclos
    float          *pStationaryRMS;       //Valores de tension estacionarios
    bool            pPrevStationary;      //Almacena el estado del flag de regimen estacionario
    Event_t         pRunningEvent;        //Evento en curso
  private :
    void  SaveParams(Params_t *param);
    void  SaveEvent(uint8_t cycle, bool end);
    float PercentChange(float in, float reference);
  protected :
    uint64_t    pPrevTimestamp;
    Work_t      pWork;
    bool        pTerminate;
  protected :
    void     AllocParams(Params_t *params, Aggreg_Type type);
    void     ResetParams(Params_t *params);
    void     ResetWork();
    void     ResetStats();
    uint32_t GetMaxSamples();
    void     SaveWorkParams();
    void     Analyze(float minimunCurrent);
    void     EventsDetect();
    void     AccumulateAggregation(Params_t *dst, Params_t *src);
    bool     FFT_Start(FFT_t *fft, float *in, Complex_t *out);
    void     FFT_Wait(FFT_t *fft);
  protected :
    static void  *FFT_Thread(void *args);
    static float  ApplyFilter(float in, Filter_t *filter);
    static float  CalcPhi(Complex_t voltage, Complex_t current);
  protected :
    virtual uint32_t SamplesCopy(float **dest, uint32_t offset, uint32_t len) = 0;
  public :
    bool         Triphase();
    uint32_t     SampleRate();
    uint8_t      NominalFreq();
    json_object *GetJSONCopy(char *series);
  public :
    CaptureInterface(uint32_t samplerate, bool triphase, uint16_t id);
    virtual ~CaptureInterface();
};
