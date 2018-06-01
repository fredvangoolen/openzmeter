/* ================================== zMeter ===========================================================================
 * File:     CaptureMono.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <libusb-1.0/libusb.h>
#include "../DeviceInterface.h"
#include "../Analyzer.h"
#include "../Tools.h"
//----------------------------------------------------------------------------------------------------------------------
#define CAPTUREV2_VID              0x0483        //Product ID
#define CAPTUREV2_PID              0x7270        //Device ID
#define CAPTUREV2_ENDPOINT_IN        0x81        //Endpoint de lectura de datos
#define CAPTUREV2_BUFFERSIZE           10        //Tamaño del buffer (10 segundos)
#define CAPTUREV2_TRANSFERS            32        //Numero de transferencias a encolar


//#define PROCESSING_FREQCYCLES          500        //Numero de ciclos usados para calcular frequencia
//#define PROCESSING_CURRENTOFFSET      0.10        //Corriente minima para realizar calculos
//----------------------------------------------------------------------------------------------------------------------
class DeviceV2 : public DeviceInterface {
  private :
    enum USBState {USB_RESET, USB_WAIT, USB_SYNC};
    enum AnalyzersState {ANALYZER_WAIT, ANALYZER_FFT, ANALYZER_PROCESSING, ANALYZER_RESET};
    typedef struct {
      uint8_t   VoltageChannel;
      uint8_t   CurrentChannel;
    } Mapping_t;
  private :
    const char                  *pSerial;                  //Numero de serie del dispositivo de captura
    int16_t                     *pBuffer;                  //Donde almaceno muestras leidas (RingBuffer)
    float                        pVoltageGain[3];          //Ganancia de tension
    float                        pCurrentGain[4];          //Ganancia de corriente
    float                        pVoltageOffset[3];        //Offset de tension
    float                        pCurrentOffset[4];        //Offset de corriente
    uint8_t                      pChannelMask;             //Mascara de canales a capturar
    uint8_t                      pChannelCount;            //Numero de canales de lectura
    uint32_t                     pADCSamplerate;           //Samplerate del ADC
    uint8_t                      pOversample;              //Oversample usado en el micro
    pthread_t                    pCaptureThread;           //Descriptor del hilo de captura
    struct libusb_device_handle *pDeviceHandle;            //Dispositivo de captura
    struct libusb_device        *pNewDevice;               //Dispositivo que se conecta
    Mapping_t                   *pMappings;                //Listado de mapeado de canales
    USBState                     pUSBState;                //Indica si la lectura del USB esta sincronizada o no
    AnalyzersState               pAnalyzerState;           //Estado de los analizadores
    
    uint32_t                     pReadPos;                 //Posicion de lectura del buffer
    uint32_t                     pWritePos;                //Posicion de escritura del buffer
    uint32_t                     pUsed;                    //Muestras en uso del buffer
    uint32_t                     pBuffLen;                 //Tamaño del buffer
    uint8_t                      pCyclesCount;             //Numero de ciclos actuales
    uint32_t                     pCyclesLen[20];           //Longitud de los ciclos detectados
    uint64_t                     pSampletime;              //Marca de tiempo del inico del bloque
  private :
    static void  *Capture_Thread(void *args);
    static void  *Analize_Thread(void *args);
    static void   ReadCallback(struct libusb_transfer *transfer);
    static int    PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
    
  private :
//    float    CalculateFrequency(Analyzer::Work_t* work);
    uint32_t SamplesCopy(float **dest, uint32_t offset, uint32_t len);
    int16_t  GetSample();
    
    void AppendSamples();
    
  public :
    DeviceV2(json_object* config);
    ~DeviceV2();
};
//----------------------------------------------------------------------------------------------------------------------
