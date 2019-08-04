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
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class FFT : private WorkerLocks {
  public :
    vector<complex<float> > Output;                //Datos de salida
  private :
    pthread_t               pThread;               //Hilo de ejecucion
    bool                    pTerminate;            //Marca para finalizar proceso
  private :
  private :
    static void *FFT_Thread(void *args);
  private :
    void   Transform(vector<complex<float> > &vec);
    void   InverseTransform(vector<complex<float> > &vec);
    void   TransformRadix2(vector<complex<float> > &vec);
    void   TransformBluestein(vector<complex<float> > &vec);
    void   Convolve(const vector<complex<float> > &xvec, const vector<complex<float> > &yvec, vector<complex<float> > &outvec);
    size_t ReverseBits(size_t x, int n);
  public :
    static float CalcAngle(const complex<float> &a, const complex<float> &b);
  public :
    FFT(char *name);
    void  Start(const vector<float> &in);
    float BinPower(uint32_t bin);
    void  Wait();
    ~FFT();
};
