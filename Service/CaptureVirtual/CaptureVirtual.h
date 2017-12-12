/* ================================== zMeter ===========================================================================
 * File:     CaptureVirtual.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "../CaptureInterface.h"
//----------------------------------------------------------------------------------------------------------------------
class CaptureVirtual : public CaptureInterface {
  private :
    typedef struct {
      float VoltageA;                                    //Valor de tension
      float VoltageB;                                    //Valor de tension
      float VoltageC;                                    //Valor de tension
      float CurrentA;                                    //Valor de corriente
      float CurrentB;                                    //Valor de corriente
      float CurrentC;                                    //Valor de corriente
      float CurrentD;                                    //Valor de corriente
    } Sample_t;
    float                        NominalVoltage;          //Tension nominal
    float                        NominalCurrent;          //Corriente nominal
    bool                         Terminate;               //Señal para detener el hilo de captura


//    uint32_t                     LastCycles[PROCESSING_FREQCYCLES];
//    uint32_t                     LastCyclesCount;
//    uint32_t                     LastCyclesPos;
//    uint32_t                     LastCyclesLenght;
//    pthread_mutex_t              AnalyzeMutex;                 //Mutex para bloquear acceso a muestras y resultados
    pthread_t                    AnalyzeThread;                //Descriptor del hilo de analisis
//    uint64_t                     PrevTimestamp;
//    uint64_t                     Agreg3S_Start;
  private : 
    static void  *Analize_Thread(void *args);
  private :
    void   SamplesCopy(float *dest, int32_t offset, uint32_t len);
  public :
    CaptureVirtual(uint32_t samplerate, bool triphase, float nominalvoltage, float nominalcurrent, CaptureInterface::Freq_t baseFreq, uint16_t id);
    ~CaptureVirtual();
};
//----------------------------------------------------------------------------------------------------------------------