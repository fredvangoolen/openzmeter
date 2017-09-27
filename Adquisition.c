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
  int16_t Current;                     // Valor de corriente
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

// Variables privadas ==================================================================================================
static Sample_t                    *Buffer;           //Donde almaceno muestras leidas (RingBuffer)
static uint32_t                     ReadPos;          //Posicion de lectura del buffer
static uint32_t                     WritePos;         //Posicion de escritura del buffer
static uint32_t                     Size;             //Tamaño total del buffer
static uint32_t                     Used;             //Muestras en uso del buffer
static pthread_mutex_t              Mutex;            //Mutex para bloquear acceso al buffer
static pthread_cond_t               WriteWait;        //Variable de condicion para esperar si buffer esta lleno
static pthread_cond_t               ReadWait;         //Variable de condicion para esperar si buffer esta vacio
static pthread_t                    Thread;           //Descriptor del hilo de captura
static bool                         Terminate;        //Señal para detener el hilo de captura
static AdquisitionFilter_t          Filters[5];       //Filtros paso banda para deteccion de paso por cero
static uint64_t                     SequenceID;       //Numero de secuencia de bloques generados
static struct libusb_device_handle *DeviceHandle;     //Dispositivo de captura

// Funciones ===========================================================================================================
void Adquisition_Callback(struct libusb_transfer *transfer) {
  int16_t *Buff16 = (int16_t*)transfer->buffer;
  int      Len = transfer->actual_length / 4;
  pthread_mutex_lock(&Mutex);
  while(Used > (Size - Len)) {
    if(Terminate) return;
    LogMsg("ERROR", "Capture buffer overflow! Posible lost of samples\n");
    pthread_cond_wait(&WriteWait, &Mutex);
  }
  for(int i = 0; i < Len; i++) {
    Buffer[WritePos].Voltage = Buff16[i * 2 + 0];
    Buffer[WritePos].Current = Buff16[i * 2 + 1];
    WritePos = (WritePos + 1) % Size;
  }
  Used += Len;
  pthread_cond_signal(&ReadWait);
  pthread_mutex_unlock(&Mutex);
  if(Terminate == false) libusb_submit_transfer(transfer);
}
void *Adquisition_Thread(void *args) {
  struct sched_param Prio;
  Prio.sched_priority = sched_get_priority_max(SCHED_OTHER);
  if(pthread_setschedparam(0, SCHED_OTHER, &Prio) == -1) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  pthread_setname_np(Thread, "Adquisition");
  uint8_t Buffer_A[8192], Buffer_B[8192];
  struct libusb_transfer *Transfer_A = libusb_alloc_transfer(0);
  struct libusb_transfer *Transfer_B = libusb_alloc_transfer(0);
  libusb_fill_bulk_transfer(Transfer_A, DeviceHandle, ADQUISITION_ENDPOINT_IN, Buffer_A,  sizeof(Buffer_A), Adquisition_Callback, NULL, 500);
  libusb_fill_bulk_transfer(Transfer_B, DeviceHandle, ADQUISITION_ENDPOINT_IN, Buffer_B,  sizeof(Buffer_B), Adquisition_Callback, NULL, 500);
  libusb_submit_transfer(Transfer_A);
  libusb_submit_transfer(Transfer_B);
  while(!Terminate) libusb_handle_events_completed(NULL, NULL);
  return NULL;
}
bool Adquisition_Start(char *device, uint32_t bufferLen, float baseFreq) {
  LogMsg("INFO", "Starting adquisition thread...\n");
  Buffer = NULL;
  ReadPos = 0;
  WritePos = 0;
  Size = bufferLen;
  Used = 0;
  SequenceID = 0;
  DeviceHandle = NULL;
  Terminate = false;
  Thread = 0;
  for(int i = 0; i < 5; i++) {
    float Q = 1.0;
    float K = tan(M_PI * (baseFreq / ADQUISITION_SAMPLEFREQ));
    float Norm = 1 / (1 + K / Q + K * K);
    Filters[i].A0 = K / Q * Norm;
    Filters[i].A1 = 0;
    Filters[i].A2 = -Filters[i].A0;
    Filters[i].B1 = 2 * (K * K - 1) * Norm;
    Filters[i].B2 = (1 - K / Q + K * K) * Norm;
    Filters[i].Z1 = 0.0;
    Filters[i].Z2 = 0.0;            
  }
  if(pthread_cond_init(&WriteWait, NULL) != 0) {                                  
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false;                                                                  
  } 
  if(pthread_cond_init(&ReadWait, NULL) != 0) {                                  
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false;                                                                  
  } 
  if(pthread_mutex_init(&Mutex, NULL) != 0) {                                  
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false;                                                                  
  } 
  int Error = libusb_init(NULL);
        if(Error < 0) {
    LogMsg("ERROR", "Fallo la apertura de libUSB...\n");
                return false;
        }
  DeviceHandle = libusb_open_device_with_vid_pid(NULL, ADQUISITION_USB_VID, ADQUISITION_PID);
        if(!DeviceHandle) {
                LogMsg("ERROR", "Dispositivo de captura no conectado...\n");
    libusb_exit(NULL);
                return false;
        }
  libusb_detach_kernel_driver (DeviceHandle, 0);
  Error = libusb_claim_interface(DeviceHandle, 0);
        if(Error < 0) {
                LogMsg("ERROR", "Dispositivo de captura ocupado...\n");
    libusb_close(DeviceHandle);
    libusb_exit(NULL);
                return false;
        }
  Buffer = (Sample_t*)malloc(bufferLen * sizeof(Sample_t));
  if(Buffer == NULL) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(NULL);
    return false; 
  }
  if(pthread_create(&Thread, NULL, Adquisition_Thread, NULL) != 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(NULL);
    free(Buffer);
    return false;
  }
  return true;
}
void Adquisition_Stop() {
  if(Buffer) {
    pthread_cond_signal(&WriteWait);
    pthread_cond_signal(&ReadWait);
    Terminate = true;
    LogMsg("INFO", "Waiting for adquisition thread ends...\n");
    pthread_join(Thread, NULL);
    free(Buffer);
  }
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
  Work_t *Work = (Work_t*)calloc(sizeof(Work_t), 1);
  if(Work == NULL) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  Work->Cycles_Len = 0;
  Work->Cycles[Work->Cycles_Len].Start = 0;
  struct timeval Time;
  pthread_mutex_lock(&Mutex);
  while(Used < (MAX_FRAME + ADQUISITION_SAFESAMPLES)) {
    if(Terminate) return NULL;
    pthread_cond_wait(&ReadWait, &Mutex);
  }
  if(gettimeofday(&Time, NULL) != 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    free(Work);
    return NULL;
  }
  Work->Timestamp = ((uint64_t)Time.tv_sec * 1000 + Time.tv_usec / 1000) - ((Used * 1000) / ADQUISITION_SAMPLEFREQ);
  float LastSample = +100; //Siempre comienzo con cierto offset para que el primer flanco sea el positivo
  for(Work->Samples = 0; Work->Samples < MAX_FRAME; Work->Samples++) {
    printf("Voltage: %d - Current: %d\n", Buffer[ReadPos].Voltage, Buffer[ReadPos].Current);
    Work->VoltageSamples[Work->Samples] = (Buffer[ReadPos].Voltage - Config.Electrical.VoltageOffset) * Config.Electrical.VoltageGain;
    Work->CurrentSamples[Work->Samples] = (Buffer[ReadPos].Current - Config.Electrical.CurrentOffset) * Config.Electrical.CurrentGain;;
    float Out = Work->VoltageSamples[Work->Samples];
    for(int i = 0; i < 5; i++) Out = Adquisition_Filter(Out, &Filters[i]);
    if(((LastSample >= 0.0) && (Out < 0.0)) || ((LastSample <= 0.0) && (Out > 0.0))) {
      Work->Cycles[Work->Cycles_Len].End = Work->Samples;
      Work->Cycles_Len++;
      if(Work->Cycles_Len == 20) break;
      Work->Cycles[Work->Cycles_Len].Start = Work->Samples;
    }
    LastSample = Out;
    ReadPos = (ReadPos + 1) % Size;
  }
  Work->SequenceID = SequenceID++;
  Used -= Work->Samples;
  pthread_cond_signal(&WriteWait);
  pthread_mutex_unlock(&Mutex);
  if(Work->Cycles_Len == 0) {
    Work->Cycles[Work->Cycles_Len].End = Work->Samples;
    Work->Cycles_Len++;
  }
  return Work;
}
void Adquisition_SamplesCopy(float *dest, int32_t offset, uint32_t len) {
  for(int i = 0; i < len; i++) 
    dest[i] = (Buffer[(ReadPos - offset + i) % Size].Voltage - Config.Electrical.VoltageOffset) * Config.Electrical.VoltageGain;
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
}
//======================================================================================================================