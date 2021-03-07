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
#include "Device.h"
#include "USBTransfer.h"
#include "DeviceV2Analyzer.h"
//----------------------------------------------------------------------------------------------------------------------
#define DEVICEV2_VID             0x0483                          //Product ID
#define DEVICEV2_PID             0x7270                          //Device ID
#define DEVICEV2_VERSION         0x0201                          //Device version
#define DEVICEV2_MANSTRING       "zRed"                          //Manufacturer string
#define DEVICEV2_PRODSTRING      "openZmeter - Capture device"   //Product string
#define DEVICEV2_ENDPOINT_IN       0x81                          //Endpoint de lectura de datos
#define DEVICEV2_ENDPOINT_OUT      0x02                          //Endpoint de escritura de datos
#define DEVICEV2_SAMPLEFREQ       24000                          //Frequencia de muestreo
#define DEVICEV2_TRANSFERS          128                          //Numero de transferencias a encolar
#define DEVICEV2_CONTROL_SETSTATUS 0x20
//----------------------------------------------------------------------------------------------------------------------
class DeviceV2 final : public Device {
  private :
    enum Button_Action { NONE = 0, CLICK = 1, LONG_CLICK = 2, VERY_LONG_CLICK = 3};
    enum LED_Color {OFF = 0, RED = 1, GREEN = 2, YELLOW = 3 };
  private :
    static bool                  pRegistered;
  private :
    uint8_t                      pSubmittedTransfers;      //Keep count of currently submitted transfers
    pthread_t                    pThread;                  //Descriptor del hilo de captura
    volatile bool                pTerminate;               //Marca que deben detenerse los trabajos pendientes
    vector<DeviceV2Analyzer*>    pAnalyzers;               //Listado de analyzadores
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
    pthread_rwlock_t             pAverageLocks;            //Lock access to average values
    bool                         pButtonAction;            //Last action readed
    uint64_t                     pLastEvent;               //Last event generated
    uint8_t                      pChannelsDelaySize[7];    //Delay line size for each channel
    uint8_t                      pChannelsDelayPos[7];     //Delay line position for each channel
    int16_t                     *pChannelsDelay;           //Channel delay line buffers
    float                        pBatteryVoltage;          //Device battery voltage
    bool                         pMainsPowered;            //Device is mains powered
    bool                         pCharging;                //Device batery is charging
  private :
    static bool  Register();
    static void  GetInstances();
    static json  CheckConfig(const json &config);
  private :
    DeviceV2(const string &serial);
    void     ClearAnalyzers();
    uint32_t AnalyzersCount();
    void     Config(const json &config);
    void     GetDevice(const json &params, Response &response) override;
    void     SetDevice(const json &params, Response &response) override;
    void    *Thread();
    void     ReadCallback(struct libusb_transfer *transfer);
    int      PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event);
  public :
    ~DeviceV2();
};
//----------------------------------------------------------------------------------------------------------------------
