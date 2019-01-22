/* ================================== zMeter ===========================================================================
 * File:     DeviceV2.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <RingBuffer.h>
#include <libusb-1.0/libusb.h>
#include "DeviceInterface.h"
#include "AnalyzerSoft.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
#define DEVICEV2_VID              0x0483        //Product ID
#define DEVICEV2_PID              0x7270        //Device ID
#define DEVICEV2_ENDPOINT_IN        0x81        //Endpoint de lectura de datos
#define DEVICEV2_TRANSFERS            32        //Numero de transferencias a encolar
#define DEVICEV2_BUFFERSECONDS        10        //Tamaño del buffer (10 segundos)
//----------------------------------------------------------------------------------------------------------------------
class DeviceV2 : public DeviceInterface {
  private :
    typedef struct {
      uint8_t       OffsetV;
      uint8_t       VoltageCount;
      uint8_t       OffsetC;
      uint8_t       CurrentCount;
      AnalyzerSoft *Analyzer;
      char         *ID;
    } Analyzer_t;
    typedef struct {
      float Voltage[3];
      float Current[4];
    } Sample_t;
    enum USBState {USB_RESET, USB_WAIT, USB_SYNC};
  private :
    vector<Analyzer_t>           pAnalyzers;               //Listado de analyzadores
//    uint8_t                      pAnalyzersCount;          //Numero de analyzadores
    RingBuffer<Sample_t>         pBuffer;                  //Read buffer
//    uint32_t                     pUsed;                    //Muestras en uso del buffer
//    uint32_t                     pBuffLen;                 //Tamaño del buffer
//    float                       *pVoltageSamples;          //Muestras de tension
//    float                       *pCurrentSamples;          //Muestras de corriente
//    uint32_t                     pReadPos;                 //Posicion de lectura del buffer
//    uint32_t                     pWritePos;                //Posicion de escritura del buffer
    uint64_t                     pLastUpdate;              //Timestamp de la ultima vez que se actualizo el nombre del thread
    char                        *pSerial;                  //Numero de serie del dispositivo de captura
    float                        pReadSamples[7];          //Muestras leidas, previas a clasificacion
    uint8_t                      pChannel;                 //Canal actual de lectura
    float                        pVoltageGain[3];          //Ganancia de tension
    float                        pCurrentGain[4];          //Ganancia de corriente
    float                        pVoltageOffset[3];        //Offset de tension
    float                        pCurrentOffset[4];        //Offset de corriente
    uint8_t                      pChannelMask;             //Mascara de canales a capturar
    uint8_t                      pChannelCount;            //Numero de canales de lectura
    uint8_t                      pVoltageChannelCount;     //Numero de canales de tension
    uint8_t                      pCurrentChannelCount;     //Numero de canales de corriente
    uint32_t                     pADCSamplerate;           //Samplerate del ADC
    uint8_t                      pOversample;              //Oversample usado en el micro
    struct libusb_device_handle *pDeviceHandle;            //Dispositivo de captura
    struct libusb_device        *pNewDevice;               //Dispositivo que se conecta
    USBState                     pUSBState;                //Indica si la lectura del USB esta sincronizada o no
  private :
    static bool                  pClassRegistered;
  private :
    static void            *Capture_Thread(void *args);
    static void             ReadCallback(struct libusb_transfer *transfer);
    static int              PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
    static DeviceInterface *CreateInstance(json_object* config);
  private :
    bool BuffersAppend();
    AnalyzerInterface *FindAnalyzer(const char *serial);
  public :
    ~DeviceV2();
};
//----------------------------------------------------------------------------------------------------------------------
