/* ================================== zMeter ===========================================================================
 * File:     FFT.h
 * Author:   Eduardo Viciana (30 de mayo de 2018)
 * ------------------------------------------------------------------------------------------------------------------ */
#pragma once
#include <vector>
#include <complex>
#include <stdint.h>
#include <pthread.h>
#include "Tools.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class AnalyzerSoft::FFT final {
  public :
    vector<complex<float> > Output;                //Datos de salida
  private :
    pthread_t               pThread;               //Hilo de ejecucion
    WorkerLocks             pWorkerLocks;          //Threadworker sync
    vector<float>           pInput;                //Muestras de entrada
    bool                    pTerminate;            //Marca para finalizar proceso
  private :
    static pthread_mutex_t  pPlannerMutex;
  private :
    static void *FFT_Thread(void *args);
  public :
    static float CalcAngle(const complex<float> &a, const complex<float> &b);
  public :
    FFT(const string &name);
    void  Start(const vector<float> &in);
    float BinPower(const uint bin);
    void  Wait();
    ~FFT();
};
