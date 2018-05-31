/* ================================== zMeter ===========================================================================
 * File:     FFT.h
 * Author:   Eduardo Viciana (30 de mayo de 2018)
 * ------------------------------------------------------------------------------------------------------------------ */
#pragma once
class FFT {
  public :
    typedef struct {
      float Real;                         //Parte real
      float Imag;                         //Parte imaginaria
    } Complex_t;
  public :
    Complex_t *Output;                    //Datos de salida
  public :
    static float CalcAngle(Complex_t a, Complex_t b);
  public :
    FFT(uint32_t maxLen);
    bool Start(float *in, uint32_t len);
    void Wait();
    ~FFT();
  private :
    float     *pInput;                    //Muestras de entrada
    uint32_t   pLen;                      //Longitud de la transformada
    fftwf_plan pPlan;                     //Plan para realizar la transformada
    pthread_t  pThread;                   //Hilo de ejecucion
  private :
    static pthread_mutex_t pPlannerMutex; //Bloquea el acceso simultaneo al plinificador de FFTs
    static void  *FFT_Thread(void *args);
};
