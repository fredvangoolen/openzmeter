/* ================================== zMeter ===========================================================================
 * File:     RequestServer.c
 * Author:   Pedro Sanchez (3 de marzo de 2015)
 * Modified: Eduardo Viciana (20 de marzo de 2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#include <stdlib.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "Main.h"
/* Escucha las peticiones que lleguen por el socket, devolviendo los datos disponibles
 * La escucha se realiza en un unico hilo y se atiende de forma secuencial, por que es de esperar que no haga 
 * demasiados clientes simultaneos */

// Tipos privados ======================================================================================================
typedef struct {
  struct pollfd   FD;
  bool            Terminate;
  pthread_t       Thread;
  pthread_mutex_t Mutex; 
  float           Samples_Voltage[MAX_FRAME];
  float           Samples_Current[MAX_FRAME];
  float           FFT_Voltage[MAX_FRAME / 2 + 1];
  float           FFT_Current[MAX_FRAME / 2 + 1];
  uint64_t        Timestamp;
  Params_t        Params;
  uint32_t        SamplesLen;
  uint32_t        FFTLen;
} RequestServer_t;

// Variables privadas ==================================================================================================
RequestServer_t RequestServer;

// Funciones ===========================================================================================================
void RequestServer_Handle(int socket) {
  char Buffer[REQUESTSERVER_BUFSIZE];
  if(recv(socket, Buffer, REQUESTSERVER_BUFSIZE, 0) == -1) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
  char *Command  = strtok(Buffer, "|");
  if(Command == NULL) return;
  char *Param1 = strtok(NULL, "|");
  if(Param1 == NULL) return;
  uint64_t Timestamp = strtoll(Param1, NULL, 10);
  pthread_mutex_lock(&RequestServer.Mutex);
  if(RequestServer.SamplesLen == 0) {
    pthread_mutex_unlock(&RequestServer.Mutex);
    return;
  }
  int Len;
  if(strcmp(Command, "OSCILLOSCOPE") == 0) {
    Len = sprintf(Buffer, "{\n  \"time\" : %jd,\n", RequestServer.Timestamp);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"points\" : [\n");
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "    %f, %f",  RequestServer.Samples_Voltage[0], RequestServer.Samples_Current[0]);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    for(int i = 1; i < RequestServer.SamplesLen; i++) {
      Len = sprintf(Buffer, ",\n    %f, %f", RequestServer.Samples_Voltage[i], RequestServer.Samples_Current[i]);
      send(socket, Buffer, Len, MSG_NOSIGNAL);
    }
    pthread_mutex_unlock(&RequestServer.Mutex);
    send(socket, "\n  ]\n}\n", 7, MSG_NOSIGNAL);
  } else if(strcmp(Command, "FFT_V") == 0) {
    Len = sprintf(Buffer, "{\"time\":%jd,\"points\":[%f", RequestServer.Timestamp, RequestServer.FFT_Voltage[0]);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    for(int i = 1; i < RequestServer.FFTLen; i++) {
      Len = sprintf(Buffer, ",%f", RequestServer.FFT_Voltage[i]);
      send(socket, Buffer, Len, MSG_NOSIGNAL);
    }
    pthread_mutex_unlock(&RequestServer.Mutex);
    send(socket, "]}", 2, MSG_NOSIGNAL);
  } else if(strcmp(Command, "FFT_I") == 0) {
    Len = sprintf(Buffer, "{\"time\":%jd,\"points\":[%f", RequestServer.Timestamp, RequestServer.FFT_Current[0]);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    for(int i = 1; i < RequestServer.FFTLen; i++) {
      Len = sprintf(Buffer, ",%f", RequestServer.FFT_Current[i]);
      send(socket, Buffer, Len, MSG_NOSIGNAL);
    }
    pthread_mutex_unlock(&RequestServer.Mutex);
    send(socket, "]}", 2, MSG_NOSIGNAL);
  } else if(strcmp(Command, "VALUES") == 0) {
    Len = sprintf(Buffer, "{\n  \"timestamp\" : %jd,\n", RequestServer.Timestamp);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"rms_v\" : %f,\n", RequestServer.Params.RMS_V);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"rms_i\" : %f,\n", RequestServer.Params.RMS_I);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"active_power\" : %f,\n", RequestServer.Params.Active_Power);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"apparent_power\" : %f,\n", RequestServer.Params.Apparent_Power);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"reactive_power\" : %f,\n", RequestServer.Params.Reactive_Power);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"power_factor\" : %f,\n", RequestServer.Params.Power_Factor);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"fundamental_active_power\" : %f,\n", RequestServer.Params.Fundamental_Active_Power);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"fundamental_apparent_power\" : %f,\n", RequestServer.Params.Fundamental_Apparent_Power);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"fundamental_reactive_power\" : %f,\n", RequestServer.Params.Fundamental_Reactive_Power);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"fundamental_power_factor\" : %f,\n", RequestServer.Params.Fundamental_Power_Factor);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"energy\" : %f,\n", RequestServer.Params.Active_Energy);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"reactive_energy\" : %f,\n", RequestServer.Params.Reactive_Energy);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"thd_v\" : %f,\n", RequestServer.Params.THD_V);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"thd_i\" : %f,\n", RequestServer.Params.THD_I);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"frequency\" : %f,\n", RequestServer.Params.Frequency);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    Len = sprintf(Buffer, "  \"harmonics_v\" : [\n    %f", RequestServer.Params.VoltageHarmonics[0]);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    for(int i = 1; i < 50; i++) {
      Len = sprintf(Buffer, ",\n    %f", RequestServer.Params.VoltageHarmonics[i]);
      send(socket, Buffer, Len, MSG_NOSIGNAL);
    }
    Len = sprintf(Buffer, "\n  ],\n  \"harmonics_i\" : [\n    %f", RequestServer.Params.CurrentHarmonics[0]);
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    for(int i = 1; i < 50; i++) {
      Len = sprintf(Buffer, ",\n    %f", RequestServer.Params.CurrentHarmonics[i]);
      send(socket, Buffer, Len, MSG_NOSIGNAL);
    }
    Len = sprintf(Buffer, "\n  ]\n}\n");
    send(socket, Buffer, Len, MSG_NOSIGNAL);
    pthread_mutex_unlock(&RequestServer.Mutex);
  } else {
    pthread_mutex_unlock(&RequestServer.Mutex);
  }
}
void *RequestServer_Thread(void *arg) {
  pthread_setname_np(RequestServer.Thread, "Request Server");
  while(!RequestServer.Terminate) {
    RequestServer.FD.revents = 0;
    int Result = poll(&RequestServer.FD, 1, 250);
    if((Result == -1) && (errno != EAGAIN)) LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    if(Result <= 0) continue;
    socklen_t PeerAddrSize = sizeof(struct sockaddr_un);
    struct sockaddr_un PeerAddr;
    int Accepted_Connection = accept(RequestServer.FD.fd, (struct sockaddr *)&PeerAddr, &PeerAddrSize);
    if(Accepted_Connection == -1) {
      if(errno != EAGAIN) LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      continue;
    }
    RequestServer_Handle(Accepted_Connection);
    close(Accepted_Connection);
  }
  return NULL;
}
bool RequestServer_Start(char *path) {
  LogMsg("INFO", "Starting request server thread...\n");
  if(pthread_mutex_init(&RequestServer.Mutex, NULL) != 0) {                                  
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false;                                                                  
  } 
  RequestServer.FD.fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
  RequestServer.FD.events = POLLIN;
  if(RequestServer.FD.fd == -1) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false;
  }
  struct sockaddr_un Server_Addr;
  memset(&Server_Addr, 0, sizeof(struct sockaddr_un));
  Server_Addr.sun_family = AF_UNIX;
  strncpy(Server_Addr.sun_path, path, sizeof(Server_Addr.sun_path) - 1);
  unlink(Server_Addr.sun_path);
  if(bind(RequestServer.FD.fd, (struct sockaddr *) &Server_Addr, sizeof(struct sockaddr_un)) == -1) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    close(RequestServer.FD.fd);
    return false;
  }
  if(chmod(path, REQUESTSERVER_PERM) == -1) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    close(RequestServer.FD.fd);
    return false; 
  }
  if(listen(RequestServer.FD.fd, REQUESTSERVER_CLIENTS) == -1) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    close(RequestServer.FD.fd);
    return false;
  }
  if(pthread_create(&RequestServer.Thread, NULL, RequestServer_Thread, NULL) != 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    close(RequestServer.FD.fd);
    return false;
  }
  RequestServer.SamplesLen = 0;
  RequestServer.FFTLen = 0;
  return true;
}
void RequestServer_Stop() {
  RequestServer.Terminate = true;
  LogMsg("INFO", "Waiting for request server thread ends...\n");
  pthread_join(RequestServer.Thread, NULL);
  close(RequestServer.FD.fd);
}
void RequestServer_SetValues(uint64_t timestamp, Params_t *params, float *voltage, float *current, float *fftv, float *ffti, uint32_t lensamples, uint32_t lenfft) {
  pthread_mutex_lock(&RequestServer.Mutex);
//  if(lensamples > RequestServer.SamplesLen) {
//    RequestServer.Samples_Voltage = realloc(RequestServer.Samples_Voltage, lensamples * sizeof(float));
//    RequestServer.Samples_Current = realloc(RequestServer.Samples_Current, lensamples * sizeof(float));
//  }
//  if(lenfft > RequestServer.FFTLen) {
//    RequestServer.FFT_Voltage = realloc(RequestServer.FFT_Voltage, lenfft * sizeof(float));
//    RequestServer.FFT_Current = realloc(RequestServer.FFT_Current, lenfft * sizeof(float));
//  }
  RequestServer.Timestamp = timestamp;
  RequestServer.SamplesLen = lensamples;
  RequestServer.FFTLen = lenfft;
  memcpy(&RequestServer.Samples_Voltage, voltage, lensamples * sizeof(float));
  memcpy(&RequestServer.Samples_Current, current, lensamples * sizeof(float));
  memcpy(&RequestServer.Params, params, sizeof(Params_t));
  //  for(int i = 0; i < lenfft; i++) {
//    RequestServer.FFT_Voltage[i] = (fftv[i] < 0.001) ? 0.001 : fftv[i];
//    RequestServer.FFT_Current[i] = (ffti[i] < 0.001) ? 0.001 : ffti[i];
//  }
//  for(int i = 0; i < lensamples; i++) {
  memcpy(&RequestServer.FFT_Voltage, fftv, lenfft * sizeof(float));
  memcpy(&RequestServer.FFT_Current, ffti, lenfft * sizeof(float));
  pthread_mutex_unlock(&RequestServer.Mutex);
}
//======================================================================================================================