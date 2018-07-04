/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "AnalyzerInterface.h"
//----------------------------------------------------------------------------------------------------------------------
class DeviceInterface {
  protected : 
    AnalyzerInterface **pAnalyzers;               //Analizadores disponibles
    uint8_t             pAnalyzersCount;          //Numero de analizadores
    pthread_t           pCaptureThread;           //Descriptor del hilo de captura 
    bool                pTerminate;               //Marca que deben detenerse los trabajos pendientes
  protected :
    void   AddAnalyzer(AnalyzerInterface *analyzer);
    DeviceInterface();
  public :
    virtual  ~DeviceInterface();
  private :
    static DeviceInterface **pDevices;
    static uint8_t           pDevicesCount;
  private :
    static void AddDeviceInterface(DeviceInterface *device);
  public :
    static bool Config(json_object *config);
    static bool GetSeriesCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesAggregCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetAnalyzersCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesNowCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
};
//----------------------------------------------------------------------------------------------------------------------
    
//    float               pSamplerate;              //Velocidad de muestreo

//    ChannelMap_t       *pChannelMappings;         //Desplazamiento de los canales de tension y corriente
//    float               pMaxCurrent;              //Corriente maxima de los sensores
//    float               pMinCurrent;              //Corriente minima de los sensores

//    bool   BuffersAlloc(uint8_t voltageChannels, uint8_t currentChannels, float samplerate);
//    void   BuffersReset();
//    bool   BuffersAppend(float *voltageSamples, float *CurrentSamples);
//    void   AddAnalyzer(AnalyzerInterface *analyzer, uint8_t offsetV, uint8_t offsetC);

//    bool      IsActive();
//    float     GetSamplerate();
//    float     GetMaxCurrent();
//    uint32_t   GetAnalyzers();
//    AnalyzerInterface *GetAnalyzers(uint32_t idx);

//    virtual bool HasAnalyzer(char *id) = 0;
    
