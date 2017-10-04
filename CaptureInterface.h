/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once

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
    //static void   CreateInstances();
