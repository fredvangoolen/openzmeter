/* ================================== zMeter ===========================================================================
 * File:     FFT.h
 * Author:   Eduardo Viciana (30 de mayo de 2018)
 * ------------------------------------------------------------------------------------------------------------------ */
#pragma once
#include <vector>
#include <stdint.h>
#include <pthread.h>
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class FFT {
  public :
    typedef struct {
      float Real;                             //Parte real
      float Imag;                             //Parte imaginaria
    } Complex_t;
  public :
    vector<Complex_t>  Output;                //Datos de salida
  private :
    vector<float>     *pInput;                //Muestras de entrada
    pthread_t          pThread;               //Hilo de ejecucion
    pthread_mutex_t    pThreadMutex;          //Mutex para la variable de condicion
    pthread_cond_t     pThreadCond;           //Variable de condicion para esperar
    bool               pTerminate;            //Marca para finalizar proceso
  private :
    static pthread_mutex_t pPlannerMutex;  //Bloquea el acceso simultaneo al plinificador de FFTs
  private :
    static void *FFT_Thread(void *args);
  private :
    void Process();
  public :
    static float CalcAngle(Complex_t a, Complex_t b);
  public :
    FFT(char *name);
    bool  Start(vector<float> *in);
    void  Wait();
    bool  IsFinished();
    float BinPower(uint32_t bin);
    ~FFT();
};
