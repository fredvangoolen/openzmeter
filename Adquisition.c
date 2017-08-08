/* ================================== zMeter ===========================================================================
 * File:     Adquisition.c
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <libusb-1.0/libusb.h>
#include "Main.h"
/* Captura de datos desde el puerto USB ================================================================================
 * La captura se realiza en un hilo distinto, con alta prioridad, que escribe en un buffer circular. 
 * Las muestras se recuperan en bloques de 10 ciclos o 200mS */

// Tipos privados ======================================================================================================
typedef struct {
  int16_t Voltage;                     //Valor de tension
  uint16_t Current;                     // Valor de corriente
} Sample_t;
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
  Sample_t                    *Buffer;           //Donde almaceno muestras leidas (RingBuffer)
  uint32_t                     ReadPos;          //Posicion de lectura del buffer
  uint32_t                     WritePos;         //Posicion de escritura del buffer
  uint32_t                     Size;             //Tamaño total del buffer
  uint32_t                     Used;             //Muestras en uso del buffer
  pthread_mutex_t              Mutex;            //Mutex para bloquear acceso al buffer
  pthread_t                    Thread;           //Descriptor del hilo de captura
  bool                         Terminate;        //Señal para detener el hilo de captura
  AdquisitionFilter_t          Filters[5];       //Filtros paso banda para deteccion de paso por cero
  uint64_t                     SequenceID;           //Numero de secuencia de bloques generados
  struct libusb_device_handle *DeviceHandle; //Dispositivo de captura
} Adquisition_t;
// Variables privadas ==================================================================================================
Adquisition_t Adquisition;

// Funciones ===========================================================================================================
void Adquisition_Callback(struct libusb_transfer *transfer) {
  int16_t *Buffer = (int16_t*)transfer->buffer;
  int      Len = transfer->actual_length / 4;
  do {
    pthread_mutex_lock(&Adquisition.Mutex);
    if(Adquisition.Used < (Adquisition.Size - Len - ADQUISITION_SAFESAMPLES)) {
      pthread_mutex_unlock(&Adquisition.Mutex);
      break;
    } else {
      pthread_mutex_unlock(&Adquisition.Mutex);
      LogMsg("ERROR", "Capture buffer overflow!\n");
      usleep(10000);
    }
  } while(1);
  for(int i = 0; i < Len; i++) {
    Adquisition.Buffer[Adquisition.WritePos].Voltage = Buffer[i * 2 + 0];
    Adquisition.Buffer[Adquisition.WritePos].Current = Buffer[i * 2 + 1];
    Adquisition.WritePos = (Adquisition.WritePos + 1) % Adquisition.Size;
  }
  pthread_mutex_lock(&Adquisition.Mutex);
  Adquisition.Used += Len;
  pthread_mutex_unlock(&Adquisition.Mutex);
  if(Adquisition.Terminate == false) libusb_submit_transfer(transfer);
}
void *Adquisition_Thread(void *args) {
  struct sched_param Prio;
  Prio.sched_priority = sched_get_priority_max(SCHED_OTHER);
  if(pthread_setschedparam(0, SCHED_OTHER, &Prio) == -1) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  pthread_setname_np(Adquisition.Thread, "Adquisition");
  uint8_t Buffer_A[8192], Buffer_B[8192];
  struct libusb_transfer *Transfer_A = libusb_alloc_transfer(0);
  struct libusb_transfer *Transfer_B = libusb_alloc_transfer(0);
  libusb_fill_bulk_transfer(Transfer_A, Adquisition.DeviceHandle, ADQUISITION_ENDPOINT_IN, Buffer_A,  sizeof(Buffer_A), Adquisition_Callback, NULL, 500);
  libusb_fill_bulk_transfer(Transfer_B, Adquisition.DeviceHandle, ADQUISITION_ENDPOINT_IN, Buffer_B,  sizeof(Buffer_B), Adquisition_Callback, NULL, 500);
  libusb_submit_transfer(Transfer_A);
  libusb_submit_transfer(Transfer_B);
  while(!Adquisition.Terminate) {
    libusb_handle_events_completed(NULL, NULL);
  }
  return NULL;
}
bool Adquisition_Start(char *device, uint32_t bufferLen, float baseFreq) {
  memset(&Adquisition, 0, sizeof(Adquisition));
  LogMsg("INFO", "Starting adquisition thread...\n");
  int Error = libusb_init(NULL);
	if(Error < 0) {
    LogMsg("ERROR", "Fallo la apertura de libUSB...\n");
		return false;
	}
  Adquisition.DeviceHandle = libusb_open_device_with_vid_pid(NULL, ADQUISITION_USB_VID, ADQUISITION_PID);
 	if(!Adquisition.DeviceHandle) {
 		LogMsg("ERROR", "Dispositivo de captura no conectado...\n");
    libusb_exit(NULL);
 		return false;
 	}
  libusb_detach_kernel_driver (Adquisition.DeviceHandle, 0);
  Error = libusb_claim_interface(Adquisition.DeviceHandle, 0);
	if(Error < 0) {
 		LogMsg("ERROR", "Dispositivo de captura ocupado...\n");
    libusb_close(Adquisition.DeviceHandle);
    libusb_exit(NULL);
 		return false;
	}
  if(pthread_mutex_init(&Adquisition.Mutex, NULL) != 0) {                                  
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false;                                                                  
  } 
  Adquisition.Buffer = malloc(bufferLen * sizeof(Sample_t));
  if(Adquisition.Buffer == NULL) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false; 
  }
  Adquisition.ReadPos = 0;
  Adquisition.WritePos = 0;
  Adquisition.Size = bufferLen;
  Adquisition.Used = 0;
  Adquisition.SequenceID = 0;
  for(int i = 0; i < 5; i++) {
    float Q = 1.0;
    float K = tan(M_PI * (baseFreq / ADQUISITION_SAMPLEFREQ));
    float Norm = 1 / (1 + K / Q + K * K);
    Adquisition.Filters[i].A0 = K / Q * Norm;
    Adquisition.Filters[i].A1 = 0;
    Adquisition.Filters[i].A2 = -Adquisition.Filters[i].A0;
    Adquisition.Filters[i].B1 = 2 * (K * K - 1) * Norm;
    Adquisition.Filters[i].B2 = (1 - K / Q + K * K) * Norm;
    Adquisition.Filters[i].Z1 = 0.0;
    Adquisition.Filters[i].Z2 = 0.0;            
  }
  if(pthread_create(&Adquisition.Thread, NULL, Adquisition_Thread, NULL) != 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    free(Adquisition.Buffer);
    return false;
  }
  return true;
}
void Adquisition_Stop() {
  //libusb_free_transfer();
  Adquisition.Terminate = true;
  LogMsg("INFO", "Waiting for adquisition thread ends...");
  pthread_join(Adquisition.Thread, NULL);
  free(Adquisition.Buffer);
//  close(Adquisition.FD.fd);
}
uint32_t Adquisition_Wait(uint32_t samples) {
  uint32_t Len;
  do {
    pthread_mutex_lock(&Adquisition.Mutex);
    Len = Adquisition.Used;
    if(Len > (samples + ADQUISITION_SAFESAMPLES)) {
      pthread_mutex_unlock(&Adquisition.Mutex);
      break;
    }
    pthread_mutex_unlock(&Adquisition.Mutex);
    usleep(10000);
  } while(1);
  return Len;
}
/* Ejecuta un filtro sobre una muestra de entrada, generando una muestra de salida */
float Adquisition_Filter(float in, AdquisitionFilter_t *filter) {
  float out = in * filter->A0 + filter->Z1;
  filter->Z1 = in * filter->A1 + filter->Z2 - filter->B1 * out;
  filter->Z2 = in * filter->A2 - filter->B2 * out;
  return out;
}
/* Devuelve una estructura de trabajo con 10 ciclos completos o si no se detectan ciclos con 236mS */
Work_t *Adquisition_GetWork() {
  Work_t *Work = calloc(sizeof(Work_t), 1);
  if(Work == NULL) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  Work->Cycles_Len = 0;
  Work->Cycles[Work->Cycles_Len].Start = 0;
  struct timeval Time;
  uint32_t Len = Adquisition_Wait(MAX_FRAME);  //Espero a tener 236mS como minimo
  if(gettimeofday(&Time, NULL) != 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    free(Work);
    return NULL;
  }
  Work->Timestamp = ((uint64_t)Time.tv_sec * 1000 + Time.tv_usec / 1000) - ((Len * 1000) / ADQUISITION_SAMPLEFREQ);
  float LastSample = +100; //Siempre comienzo con cierto offset para que el primer flanco sea el positivo
  for(Work->Samples = 0; Work->Samples < MAX_FRAME; Work->Samples++) {
    Work->VoltageSamples[Work->Samples] = (Adquisition.Buffer[Adquisition.ReadPos].Voltage - ADQUISITION_VOLT_OFFSET) * ADQUISITION_VOLT_GAIN;
    Work->CurrentSamples[Work->Samples] = (Adquisition.Buffer[Adquisition.ReadPos].Current - ADQUISITION_CURR_OFFSET) * ADQUISITION_CURR_GAIN;;
    float Out = Work->VoltageSamples[Work->Samples];
    for(int i = 0; i < 5; i++) Out = Adquisition_Filter(Out, &Adquisition.Filters[i]);
    if(((LastSample >= 0.0) && (Out < 0.0)) || ((LastSample <= 0.0) && (Out > 0.0))) {
      Work->Cycles[Work->Cycles_Len].End = Work->Samples;
      Work->Cycles_Len++;
      if(Work->Cycles_Len == 20) break;
      Work->Cycles[Work->Cycles_Len].Start = Work->Samples;
    }
    LastSample = Out;
    Adquisition.ReadPos = (Adquisition.ReadPos + 1) % Adquisition.Size;
  }
  Work->SequenceID = Adquisition.SequenceID++;
  pthread_mutex_lock(&Adquisition.Mutex);
  Adquisition.Used -= Work->Samples;
  pthread_mutex_unlock(&Adquisition.Mutex);
  if(Work->Cycles_Len == 0) {
    Work->Cycles[Work->Cycles_Len].End = Work->Samples;
    Work->Cycles_Len++;
  }
  return Work;
}
void Adquisition_SamplesCopy(float *dest, int32_t offset, uint32_t len) {
  for(int i = 0; i < len; i++) 
    dest[i] = (Adquisition.Buffer[(Adquisition.ReadPos - offset + i) % Adquisition.Size].Voltage - ADQUISITION_VOLT_OFFSET) * ADQUISITION_VOLT_GAIN;
}
void Adquisition_SetLEDs(float voltage, float current) {
  uint8_t Val = 0;
  int Voltage = round(((voltage / EVENT_VOLTAGE) - 1.0) * 50.0 + 4.0);
  if(Voltage < 0) Voltage = 0;
  if(Voltage > 8) Voltage = 8;
  int Current = round((current / MAX_CURRENT) * 100.0 / 9.0);
  if(Current < 0) Current = 0;
  if(Current > 8) Current = 8;
  Val = (Current << 4) + Voltage;
//  write(Adquisition.FD.fd, &Val, 1);
}
//======================================================================================================================