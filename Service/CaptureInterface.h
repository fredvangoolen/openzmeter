/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
class CaptureInterface {
  public :
    typedef struct {
      uint32_t Start;                       //Muestra dentro del bloque que inicia el ciclo
      uint32_t End;                         //Muestra dentro del bloque que finaliza el ciclo
    } Cycle_t;
    typedef struct Work_t {
      uint64_t  SequenceID;                 //Numero de secuencia del bloque
      uint64_t  Timestamp;                  //Timestamp de la muestra inicial
      uint32_t  Samples;                    //Cantidad de muestras que componen el bloque
      float    *VoltageSamples;             //Muestras de tension   (El array almacena hasta 236mS)
      float    *CurrentSamples;             //Muestras de corriente (El array almacena hasta 236mS) 
      Cycle_t   Cycles[20];                 //Informacion de los ciclos del bloque
      uint8_t   Cycles_Len;                 //Numero de ciclos en el bloque
    } Work_t;
    typedef struct {
      float Real;                           //Parte real
      float Imag;                           //Parte imaginaria
    } Complex_t;
    typedef struct {
      float      RMS_V;                       //Valor RMS de la tension durante el bloque
      float      RMS_I;                       //Valor RMS de la intensidad durante el bloque
      float      Active_Power;                //Potencia activa (P)
      float      Apparent_Power;              //Potencia aparente (S)
      float      Power_Factor;                //Factor de potencia (cos(PHI))
      float      Fundamental_Active_Power;    //Potencia activa del primer armonico (P_1)
      float      Fundamental_Apparent_Power;  //Potencia aparente fundamental (S_1)
      float      Fundamental_Reactive_Power;  //Potencia reactiva fundamental (Q_1)
      float      Fundamental_Power_Factor;    //Fundamental power factor
      float      Fundamental_Reactive_Energy; //Energia reactiva
      float      Active_Energy;               //Energia consumida
      float      THD_V;                       //Distorsion harmonica de la tension
      float      THD_I;                       //Distorsion harmonica de la corriente
      float      VoltageHarmonics[50];        //Armonicos del canal de tension
      float      CurrentHarmonics[50];        //Armonicos del canal de corriente
      float      Frequency;                   //Frecuencia
      float      Phi;                         //Angulo de Phi
      bool       Flag;                        //Periodo marcado
    } Params_t;
  public :
    virtual uint8_t CurrentChannels() = 0;
    virtual uint8_t VoltageChannels() = 0;
  public :
    bool ExecDBSentence(const char *sql);
    
    //static void   CreateInstances();
};