/* ================================== zMeter ===========================================================================
 * File:     FFT.h
 * Author:   Eduardo Viciana (30 de mayo de 2018)
 * ------------------------------------------------------------------------------------------------------------------ */
#pragma once
#include <vector>
#include <complex>
#include <stdint.h>
#include <pthread.h>
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class FFT {
  public :
    vector<complex<float> > Output;                //Datos de salida
  private :
    pthread_t               pThread;               //Hilo de ejecucion
    pthread_mutex_t         pThreadMutex;          //Mutex para la variable de condicion
    pthread_cond_t          pThreadCond;           //Variable de condicion para esperar
    bool                    pTerminate;            //Marca para finalizar proceso
    bool                    pWorking;              //Variable to check processing
  private :
  private :
    static void *FFT_Thread(void *args);
  private :
    void   Process();
    void   Transform(vector<complex<float> > &vec);
    void   InverseTransform(vector<complex<float> > &vec);
    void   TransformRadix2(vector<complex<float> > &vec);
    void   TransformBluestein(vector<complex<float> > &vec);
    void  Convolve(const vector<complex<float> > &xvec, const vector<complex<float> > &yvec, vector<complex<float> > &outvec);
    size_t ReverseBits(size_t x, int n);
  public :
    static float CalcAngle(complex<float> a, complex<float> b);
    static float BinPower(complex<float> bin);
  public :
    FFT(char *name);
    void  Start(vector<float> &in);
    void  Wait();
    ~FFT();
};
