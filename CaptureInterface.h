/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <pthread.h>
#include <inttypes.h>
#include <fftw3.h>
#include "Database.h"
//---------------------------------------------------------------------------------------------------------------------
#define PROCESSING_FREQCYCLES          500        //Numero de ciclos usados para calcular frequencia
//---------------------------------------------------------------------------------------------------------------------
class CaptureInterface : private Database {
  public :
    enum Freq_t {FREQ_50, FREQ_60};
    typedef struct {
      uint32_t Start;                       //Muestra dentro del bloque que inicia el ciclo
      uint32_t End;                         //Muestra dentro del bloque que finaliza el ciclo
    } Cycle_t;
    typedef struct {
      uint64_t  Timestamp;                  //Timestamp de la muestra inicial
      float    *RMS_V;                      //Valor RMS de la tension durante el bloque
      float    *RMS_I;                      //Valor RMS de la intensidad durante el bloque
      float    *Active_Power;               //Potencia activa (P)
      float    *Apparent_Power;             //Potencia aparente (S)
      float    *Power_Factor;               //Factor de potencia (cos(PHI))
      float    *Fundamental_Active_Power;   //Potencia activa del primer armonico (P_1)
      float    *Fundamental_Apparent_Power; //Potencia aparente fundamental (S_1)
      float    *Fundamental_Reactive_Power; //Potencia reactiva fundamental (Q_1)
      float    *Fundamental_Power_Factor;   //Fundamental power factor
      float    *Fundamental_Reactive_Energy;//Energia reactiva
      float    *Active_Energy;              //Energia consumida
      float    *THD_V;                      //Distorsion harmonica de la tension
      float    *THD_I;                      //Distorsion harmonica de la corriente
      float   **VoltageHarmonics;           //Armonicos del canal de tension
      float   **CurrentHarmonics;           //Armonicos del canal de corriente
      float    Frequency;                   //Frecuencia
      float   *Phi;                         //Angulo de Phi
      bool     Flag;                        //Periodo marcado
    } Params_t;
    typedef struct {
      uint64_t  SequenceID;                 //Numero de secuencia del bloque
      uint32_t  Samples;                    //Cantidad de muestras que componen el bloque
      float   **VoltageSamples;             //Muestras de tension   (El array almacena hasta 236mS)
      float   **CurrentSamples;             //Muestras de corriente (El array almacena hasta 236mS) 
      Cycle_t   Cycles[20];                 //Informacion de los ciclos del bloque
      uint8_t   Cycles_Len;                 //Numero de ciclos en el bloque
      Params_t  Params;
    } Work_t;
    typedef struct {
      Params_t Params;                      //Media/RMS de los valores agregados
      Params_t Max;                         //Maximo valor alcanzado en la agregacion
      Params_t Min;                         //Minimo valor alcanzado en la agregacion
      uint32_t Samples;                     //Numero de muestras incluidas en la agregacion
    } Aggreg_t;
    typedef struct {
      float Real;                           //Parte real
      float Imag;                           //Parte imaginaria
    } Complex_t;
    typedef struct {
      float     *Input;                     //Muestras de entrada
      uint32_t   Len;                       //Longitud de la transformada
      fftwf_plan Plan;                      //Plan para realizar la transformada
      pthread_t  Thread;                    //Hilo de ejecucion
      Complex_t  Result[1];                 //Datos de salida
    } FFT_t;
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
    uint32_t    pLastCycles[PROCESSING_FREQCYCLES];
    uint32_t    pLastCyclesCount;
    uint32_t    pLastCyclesPos;
    uint32_t    pLastCyclesLenght;
    uint32_t    pSampleRate;
    uint32_t    pMaxSamples;
    enum Freq_t pNominalFreq;
    bool        pTriphase;
  protected :
    Work_t      pWork;
    Aggreg_t    pAgreg3S;
    Aggreg_t    pAgreg1M;
    Aggreg_t    pAgreg10M;
    Aggreg_t    pAgreg1H;
  protected :
    bool     AllocParams(Params_t *params);
    void     ResetParams(Params_t *params);
    uint8_t  NominalFrequency();
    void     ResetWork();
    uint32_t GetMaxSamples();
    bool     SaveWorkParams();
    void     Analyze(float minimunCurrent);
  protected :
    static void  *FFT_Thread(void *args);
    static float  ApplyFilter(float in, Filter_t *filter);
    static void   FFT_Wait(FFT_t *fft);
    static FFT_t *FFT_Start(float *data, uint32_t len);
    static float  CalcPhi(Complex_t voltage, Complex_t current);
  public :
    bool        Triphase();
    uint32_t    SampleRate();
    uint8_t     NominalFreq();
  public :
    CaptureInterface(uint32_t samplerate, bool triphase, enum Freq_t freq);
    virtual ~CaptureInterface();
};
