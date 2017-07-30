#pragma once
#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>
#include <poll.h>

/* Captura de datos desde el puerto USB ===========================================================================================================================================
 * La captura se realiza en un hilo distinto, con alta prioridad, que escribe en un buffer
 * Pueden obtenerse una cantidad de muestras arbitraria, bloqueando el hilo que llama hasta que las muestras esten
 * disponibles, al extraer muestras liberamos espacio del buffer. Todas las muestras recuperadas tienen un valor
 * real del valor leido, no necesitan transformarse posteriormente */

// Real_Value = (Read_Value - Offset) * Gain
//#define ADQUISITION_VOLT_OFFSET 750.00
//#define ADQUISITION_VOLT_GAIN     0.50
//#define ADQUISITION_CURR_OFFSET 750.00
//#define ADQUISITION_CURR_GAIN     0.50

typedef struct {
  uint16_t Voltage;
  uint16_t Current;
} AdquisitionSample_t;

typedef struct {
  float A0;
  float A1;
  float A2;
  float B1;
  float B2;
  float Z1;
  float Z2;
} AdquisitionFilter_t;

typedef struct {
  AdquisitionSample_t  *Buffer;
  uint32_t              ReadPos;
  uint32_t              WritePos;
  uint32_t              Size;
  uint32_t              Used;
  bool                  Terminate;
  pthread_mutex_t       Mutex; 
  pthread_t             Thread;
  struct pollfd         FD;
  AdquisitionFilter_t   Filters[5];
  uint64_t              SequenceID;
  uint64_t              ProcessedSamples;
  float                 LastSample;
} Adquisition_t;

extern Adquisition_t Adquisition;

bool Adquisition_Start(char *device, uint32_t bufferLen, float baseFreq);
void Adquisition_Stop();
void Adquisition_Get(float *voltage, float *current, uint32_t samples);