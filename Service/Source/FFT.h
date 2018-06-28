/* ================================== zMeter ===========================================================================
 * File:     FFT.h
 * Author:   Eduardo Viciana (30 de mayo de 2018)
 * ------------------------------------------------------------------------------------------------------------------ */
#pragma once
#include <stdint.h>
#include <pthread.h>
class FFT {
  public :
    typedef struct {
      float Real;                         //Parte real
      float Imag;                         //Parte imaginaria
    } Complex_t;
  public :
    Complex_t *Output;                    //Datos de salida
    uint32_t   OutputLen;                 //Longitud de salida
  private :
    float          *pInput;                //Muestras de entrada
    uint32_t        pLen;                  //Longitud de la transformada
    pthread_t       pThread;               //Hilo de ejecucion
    pthread_mutex_t pThreadMutex;          //Mutex para la variable de condicion
    pthread_cond_t  pThreadCond;           //Variable de condicion para esperar
    bool            pTerminate;            //Marca para finalizar proceso
  private :
    static pthread_mutex_t pPlannerMutex;  //Bloquea el acceso simultaneo al plinificador de FFTs
    static void  *FFT_Thread(void *args);
  private :
    void Process();
  public :
    static float CalcAngle(Complex_t a, Complex_t b);
  public :
    FFT(uint32_t maxLen, char *name);
    bool  Start(float *in, uint32_t len);
    void  Wait();
    bool  IsFinished();
    float BinPower(uint32_t bin);
    ~FFT();
};
