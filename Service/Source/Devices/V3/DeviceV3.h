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
#include "DeviceV3Analyzer.h"
//----------------------------------------------------------------------------------------------------------------------
#define DEVICEV3_VID             0x0483                          //Product ID
#define DEVICEV3_PID             0x7270                          //Device ID
#define DEVICEV3_VERSION         0x0300                          //Device version
#define DEVICEV3_MANSTRING       "zRed"                          //Manufacturer string
#define DEVICEV3_PRODSTRING      "openZmeter - Capture device"   //Product string
#define DEVICEV3_ENDPOINT_IN       0x81                          //Endpoint de lectura de datos
#define DEVICEV3_SAMPLEFREQ       24000                          //Frequencia de muestreo
#define DEVICEV3_TRANSFERS          128                          //Numero de transferencias a encolar
//----------------------------------------------------------------------------------------------------------------------
class DeviceV3 final : public Device {
  private :
    static bool                  pRegistered;
  private :
    uint8_t                      pSubmittedTransfers;      //Keep count of currently submitted transfers
    pthread_t                    pThread;                  //Descriptor del hilo de captura
    volatile bool                pTerminate;               //Marca que deben detenerse los trabajos pendientes
    DeviceV3Analyzer            *pAnalyzers[9];            //Listado de analizadores
    uint8_t                      pAnalyzersCount;          //Running analyzers count
    pthread_mutex_t              pConfigLock;              //Block configuration params
    float                        pVoltageGain;             //Ganancia de tension
    float                        pCurrentGain[8];          //Ganancia de corriente
    float                        pVoltageOffset;           //Offset de tension
    float                        pCurrentOffset[8];        //Offset de corriente
    int16_t                      pCapture[9];              //Captured samples
    uint8_t                      pCaptureSize;             //Captured samples count
    libusb_device_handle        *pDeviceHandle;            //Dispositivo de captura
    libusb_device               *pNewDevice;               //Dispositivo que se conecta
    libusb_context              *pContext;                 //USB context
    int32_t                      pRunAverageValues[9];     //Average wave value of last second
    uint32_t                     pRunAverageCounter;       //Somple count for average
    float                        pAverageValues[9];        //Average wave value of last second
    pthread_rwlock_t             pAverageLocks;            //Lock access to average values
    uint8_t                      pChannelsDelaySize[9];    //Delay line size for each channel
    uint8_t                      pChannelsDelayPos[9];     //Delay line position for each channel
    int16_t                     *pChannelsDelay;           //Channel delay line buffers
  private :
    static bool  Register();
    static void  GetInstances();
    static json  CheckConfig(const json &config);
  private :
    DeviceV3(const string &serial);
    void     ClearAnalyzers();
    uint32_t AnalyzersCount();
    void     Config(const json &config);
    void     GetDevice(HTTPRequest &req) override;
    void     SetDevice(HTTPRequest &req) override;
    void    *Thread();
    void     ReadCallback(struct libusb_transfer *transfer);
    int      PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event);
  public :
    ~DeviceV3();
};
//----------------------------------------------------------------------------------------------------------------------
