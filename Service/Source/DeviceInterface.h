/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "AnalyzerInterface.h"
#include "FFT.h"
//----------------------------------------------------------------------------------------------------------------------
#define DEVICEINTERFACE_BUFFERSECONDS     10        //Tamaño del buffer (10 segundos)
//----------------------------------------------------------------------------------------------------------------------
class DeviceInterface {
  private :
//    typedef struct {
//      uint8_t OffsetV;
//      uint8_t OffsetC;
//    } ChannelMap_t;
  private :

//    uint8_t             pVoltageChannels;         //Canales de tension
//    uint8_t             pCurrentChannels;         //Canales de corriente
//    float              *pVoltageSamples;          //Muestras de tension
//    float              *pCurrentSamples;          //Muestras de corriente
//    uint32_t            pReadPos;                 //Posicion de lectura del buffer
//    uint32_t            pWritePos;                //Posicion de escritura del buffer
//    uint32_t            pUsed;                    //Muestras en uso del buffer
//    uint32_t            pBuffLen;                 //Tamaño del buffer
//    float               pSamplerate;              //Velocidad de muestreo
    AnalyzerInterface **pAnalyzers;               //Analizadores disponibles
    uint8_t             pAnalyzersCount;          //Numero de analizadores
//    ChannelMap_t       *pChannelMappings;         //Desplazamiento de los canales de tension y corriente
//    float               pMaxCurrent;              //Corriente maxima de los sensores
//    float               pMinCurrent;              //Corriente minima de los sensores
//    uint64_t            pLastUpdate;              //Timestamp de la ultima vez que se actualizo el nombre del thread
  protected :
//    char                pDriverName[16];            //Nombre del driver (para mostrar en el thread)
    pthread_t           pCaptureThread;             //Descriptor del hilo de captura
//    bool                pActive;                  //Indicador de actividad de la captura
    bool                pTerminate;               //Marca que deben detenerse los trabajos pendientes
  protected :
//    bool   BuffersAlloc(uint8_t voltageChannels, uint8_t currentChannels, float samplerate);
//    void   BuffersReset();
//    bool   BuffersAppend(float *voltageSamples, float *CurrentSamples);
//    void   AddAnalyzer(AnalyzerInterface *analyzer);
//    void   AddAnalyzer(AnalyzerInterface *analyzer, uint8_t offsetV, uint8_t offsetC);
    DeviceInterface();
  public :
    virtual  ~DeviceInterface();
//    bool      IsActive();
//    float     GetSamplerate();
//    float     GetMaxCurrent();
//    uint32_t   GetAnalyzers();
//    AnalyzerInterface *GetAnalyzers(uint32_t idx);


  
  private :
    static DeviceInterface **pDeviceInterfaces;
    static uint8_t           pDeviceInterfacesCount;
  private :
    static void AddDeviceInterface(DeviceInterface *device);
  public :
    static void Config(json_object *config);
    static bool GetSeriesCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesAggregCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetAnalyzersCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesNowCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
};
//----------------------------------------------------------------------------------------------------------------------