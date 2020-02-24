// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
#include <libusb-1.0/libusb.h>
#include <deque>
#include "Device.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
#define DEVICEV2_VID             0x0483                          //Product ID
#define DEVICEV2_PID             0x7270                          //Device ID
#define DEVICEV2_VERSION         0x0201                          //Device version
#define DEVICEV2_MANSTRING       "zRed"                          //Manufacturer string
#define DEVICEV2_PRODSTRING      "openZmeter - Capture device"   //Product string
#define DEVICEV2_ENDPOINT_IN       0x81                          //Endpoint de lectura de datos
#define DEVICEV2_ENDPOINT_OUT      0x02                          //Endpoint de escritura de datos
#define DEVICEV2_SAMPLEFREQ       24000                          //Frequencia de muestreo
#define DEVICEV2_TRANSFERS           16                          //Numero de transferencias a encolar
#define DEVICEV2_BUFFERSIZE    0x040000
#define DEVICEV2_CONTROL_SETSTATUS 0x20
//----------------------------------------------------------------------------------------------------------------------
class DeviceV2 final : public Device {
  private :
    enum Button_Action { NONE = 0, CLICK = 1, LONG_CLICK = 2, VERY_LONG_CLICK = 3};
    enum LED_Color {OFF = 0, RED = 1, GREEN = 2, YELLOW = 3 };
    typedef struct {
      AnalyzerSoft           *Parent;
      uint8_t                 Voltage[3];
      uint8_t                 VoltageChannels;
      uint8_t                 Current[4];
      float                   CurrentInvert[4];
      uint8_t                 CurrentChannels;
      Analyzer::AnalyzerMode  Mode;
    } Analyzer_t;
    class Transfer {
      private :
        DeviceV2        *pParent;
        libusb_transfer *pTransfer;
        uint8_t         *pBuffer;
        uint32_t         pBufferSize;
      public :
        Transfer();
        void Init(DeviceV2 *parent, uint32_t size);
        ~Transfer();
        int      Submit();
        int      Submit(void *data, uint32_t size);
        int      Submit(libusb_device_handle *deviceHandle, uint8_t endpoint, libusb_transfer_cb_fn callback, int32_t timeout);
        void     Setup(libusb_device_handle *deviceHandle, uint8_t endpoint, libusb_transfer_cb_fn callback, int32_t timeout);
        uint32_t BufferSize();
    };
  private :
    static bool                  pRegistered;
  private :
    uint8_t                      pSubmittedTransfers;      //Keep count of currently submitted transfers
    pthread_t                    pThread;                  //Descriptor del hilo de captura
    volatile bool                pTerminate;               //Marca que deben detenerse los trabajos pendientes
    vector<Analyzer_t>           pAnalyzersSetup;          //Listado de analyzadores
    int16_t                     *pBuffer;                  //Read buffer
    uint32_t                     pBufferTail;              //pBuffer read pos
    uint32_t                     pBufferHead;              //pBuffer write pos
    pthread_mutex_t              pConfigLock;              //Block configuration params
    float                        pVoltageGain[3];          //Ganancia de tension
    float                        pCurrentGain[4];          //Ganancia de corriente
    float                        pVoltageOffset[3];        //Offset de tension
    float                        pCurrentOffset[4];        //Offset de corriente
    bool                         pHighGain;                //Choose high gain mode
    int16_t                      pCapture[7];              //Captured samples
    uint8_t                      pCaptureSize;             //Captured samples count
    libusb_device_handle        *pDeviceHandle;            //Dispositivo de captura
    libusb_device               *pNewDevice;               //Dispositivo que se conecta
    libusb_context              *pContext;                 //USB context
    int32_t                      pRunAverageValues[7];     //Average wave value of last second
    uint32_t                     pRunAverageCounter;       //Somple count for average
    float                        pAverageValues[7];        //Average wave value of last second
    RWLocks                      pAverageLocks;            //Lock access to average values
    uint64_t                     pLastSignalRecv;          //Last battery message recv
    bool                         pButtonAction;            //Last action readed
    Transfer                     pTransfers[DEVICEV2_TRANSFERS];
    Transfer                     pSignalTransfer;
  private :
    static bool            Register();
    static vector<Device*> GetInstances();
    static void           *Capture_Thread(void *args);
    static void            ReadCallback(struct libusb_transfer *transfer);
    static int             PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data);
    static json            CheckConfig(const json &config);
  private :
    DeviceV2(const string &serial);
    void Configure(const json &config);
    void GetDevice(const json &params, Response &response) override;
    void SetDevice(const json &params, Response &response) override;
    void BufferAppend();
  public :
    ~DeviceV2();
};
//----------------------------------------------------------------------------------------------------------------------
