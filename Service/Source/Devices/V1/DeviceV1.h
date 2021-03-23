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
#define DEVICEV1_VID             0x0483                          //Product ID
#define DEVICEV1_PID             0x7270                          //Device ID
#define DEVICEV1_MANSTRING       "zRed"                          //Manufacturer string
#define DEVICEV1_01_PRODSTRING   "openZmeter - Capture device"   //Product string
#define DEVICEV1_ENDPOINT_IN       0x81                          //Endpoint de lectura de datos
#define DEVICEV1_SAMPLEFREQ       15625                          //Frequencia de muestreo
#define DEVICEV1_BUFFERSECONDS       10                          //Tamaño del buffer (10 segundos)
#define DEVICEV1_TRANSFERS            8                          //Numero de transferencias a encolar
#define DEVICEV1_VOLTAGEDELAY         2                          //Samples to delay
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class DeviceV1 final : public Device {
  private :
    static bool                  pRegistered;
  private :
    uint8_t                      pSubmittedTransfers;      //Keep count of currently submitted transfers
    uint16_t                     pHWVersion;               //Version de hardware
    pthread_t                    pThread;                  //Descriptor del hilo de captura
    volatile bool                pTerminate;               //Marca que deben detenerse los trabajos pendientes
    pthread_mutex_t              pConfigLock;              //Block configuration params
    float                        pVoltageGain;             //Voltage gain
    float                        pCurrentGain;             //Current gain
    float                        pVoltageOffset;           //Voltage offset
    float                        pCurrentOffset;           //Current offset
    bool                         pCurrentInvert;           //Invert current channel
    struct libusb_device_handle *pDeviceHandle;            //USB handle
    struct libusb_device        *pNewDevice;               //USB device
    libusb_context              *pContext;                 //USB context
    int32_t                      pRunAverageValues[2];     //Average wave value of last second
    uint32_t                     pRunAverageCounter;       //Somple count for average
    float                        pAverageValues[2];        //Average wave value of last second
    pthread_rwlock_t             pAverageLocks;            //Lock access to average values
    AnalyzerSoft                *pAnalyzer;
    uint8_t                      pVoltageDelayPos;
    int16_t                      pVoltageDelay[DEVICEV1_VOLTAGEDELAY];
  private :
    static bool  Register();
    static void  GetInstances();
    static json  CheckConfig(const json &config);
  private :
    DeviceV1(const string &serial, const uint16_t version);
    void    *Thread();
    void     ClearAnalyzers();
    void     Config(const json &config);
    uint32_t AnalyzersCount();
    void     ReadCallback(struct libusb_transfer *transfer);
    int      PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event);
    void     GetDevice(HTTPRequest *req) override;
    void     SetDevice(HTTPRequest *req) override;
  public :
    ~DeviceV1();
};
//----------------------------------------------------------------------------------------------------------------------
