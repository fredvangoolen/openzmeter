#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include "Main.h"
// Tipos privados ======================================================================================================
static uint32_t        LastCycles[PROCESSING_FREQCYCLES];
static uint32_t        LastCyclesCount  = 0;
static uint32_t        LastCyclesPos    = 0;
static uint32_t        LastCyclesLenght = 0;
static pthread_mutex_t Mutex;                 //Mutex para bloquear acceso a muestras y resultados
static pthread_t       Thread;                //Descriptor del hilo de analisis
static bool            Terminate;             //Señal para detener el hilo de captura
static Work_t         *Work;                  //Current working samples
static Params_t       *Params;                //Current working params
// Funciones ===========================================================================================================
float CalcPhi(Complex_t voltage, Complex_t current) {
  float Tmp;
  if((voltage.Real == 0) && (voltage.Imag == 0)) 
    Tmp = 0;
  else if(voltage.Real < 0)
    Tmp = atan(voltage.Imag / voltage.Real);
  else
    Tmp = atan(voltage.Imag / voltage.Real) + M_PI;
  
  if((current.Real == 0) && (current.Imag == 0)) 
    return Tmp;
  else if(current.Real < 0)
    return Tmp - atan(current.Imag / current.Real);
  else
    return Tmp - atan(current.Imag / current.Real) + M_PI;
}
float Analyzer_Frequency(Work_t* work) {
  for(int i = 0; i < work->Cycles_Len; i++) {
    if(LastCyclesCount == PROCESSING_FREQCYCLES) LastCyclesLenght = LastCyclesLenght - LastCycles[LastCyclesPos];
    LastCycles[LastCyclesPos] = work->Cycles[i].End - work->Cycles[i].Start;
    LastCyclesLenght = LastCyclesLenght + LastCycles[LastCyclesPos];
    LastCyclesPos = (LastCyclesPos + 1) % PROCESSING_FREQCYCLES;
    if(LastCyclesCount < PROCESSING_FREQCYCLES) LastCyclesCount++;
  }
  return LastCyclesCount / (LastCyclesLenght / ADQUISITION_SAMPLEFREQ) / 2.0;
}
bool Analyzer_Start() {
//  if(Adquisition_Start(NULL, ADQUISITION_BUFFERSIZE, 50.0) == false) return false;
  LogMsg("INFO", "Starting analyzer thread...\n");
  Terminate = false;
  Thread = 0;
  LastCyclesCount  = 0;
  LastCyclesPos    = 0;
  LastCyclesLenght = 0;
  if(pthread_mutex_init(&Mutex, NULL) != 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    Adquisition_Stop();
    return false;                                                                  
  } 
  if(pthread_create(&Thread, NULL, Analyzer_Thread, NULL) != 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    Adquisition_Stop();
    return false;
  }
  return true;
}
void Analyzer_Stop() {
  if(Thread != 0) {
    //  pthread_cond_signal(&WriteWait);
  //  pthread_cond_signal(&ReadWait);
    Terminate = true;
    LogMsg("INFO", "Waiting for analysis thread ends...\n");
    pthread_join(Thread, NULL);
    Adquisition_Stop();
  //  free(Buffer);
  }
}
