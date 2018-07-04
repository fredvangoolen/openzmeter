/* ================================== zMeter ===========================================================================
 * File:     CaptureMono.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <libusb-1.0/libusb.h>
#include "DeviceInterface.h"
#include "AnalyzerInterface.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
#define CAPTUREV2_VID              0x0483        //Product ID
#define CAPTUREV2_PID              0x7270        //Device ID
#define CAPTUREV2_ENDPOINT_IN        0x81        //Endpoint de lectura de datos
#define CAPTUREV2_TRANSFERS            32        //Numero de transferencias a encolar


//#define PROCESSING_FREQCYCLES          500        //Numero de ciclos usados para calcular frequencia
//#define PROCESSING_CURRENTOFFSET      0.10        //Corriente minima para realizar calculos
//----------------------------------------------------------------------------------------------------------------------
class DeviceV2 : public DeviceInterface {
private :
//    typedef struct {
//      uint8_t OffsetV;
//      uint8_t OffsetC;
//    } ChannelMap_t;
  private :
    enum USBState {USB_RESET, USB_WAIT, USB_SYNC};
  private :
    char                        *pSerial;                  //Numero de serie del dispositivo de captura
    float                        pReadSamples[7];          //Muestras leidas, previas a clasificacion
    uint8_t                      pChannel;                 //Canal actual de lectura
    float                        pVoltageGain[3];          //Ganancia de tension
    float                        pCurrentGain[4];          //Ganancia de corriente
    float                        pVoltageOffset[3];        //Offset de tension
    float                        pCurrentOffset[4];        //Offset de corriente
    uint8_t                      pChannelMask;             //Mascara de canales a capturar
    uint8_t                      pChannelCount;            //Numero de canales de lectura
    uint32_t                     pADCSamplerate;           //Samplerate del ADC
    uint8_t                      pOversample;              //Oversample usado en el micro
    struct libusb_device_handle *pDeviceHandle;            //Dispositivo de captura
    struct libusb_device        *pNewDevice;               //Dispositivo que se conecta
    USBState                     pUSBState;                //Indica si la lectura del USB esta sincronizada o no
  private :
    static void  *Capture_Thread(void *args);
    static void   ReadCallback(struct libusb_transfer *transfer);
    static int    PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
  private :
    bool ReadJSONFloatArray(json_object *object, float *dst, uint8_t len);
    bool ReadOutput(json_object *object, bool parseOnly);
  public :
    DeviceV2(json_object* config);
    ~DeviceV2();
};
//----------------------------------------------------------------------------------------------------------------------
