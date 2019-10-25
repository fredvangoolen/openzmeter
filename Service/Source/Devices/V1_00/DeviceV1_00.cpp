/* ================================== zMeter ===========================================================================
 * File:     CaptureMono.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <termios.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "DeviceV1_00.h"
#include "DeviceManager.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV1_00::pRegistered = DeviceV1_00::GetInstances();
// Funciones ===========================================================================================================
bool DeviceV1_00::GetInstances() {
  libusb_context *Context;
  if(libusb_init(&Context) < 0) {
    LOGFile::Error("Can not open libUSB...\n");
    return false;
  }
  libusb_device **Devs;
  int Count = libusb_get_device_list(NULL, &Devs);
  if(Count < 0) {
    LOGFile::Error("Can not enumerate USB devices...\n");
    return false;
  }
  for(int i = 0; i < Count; ++i) {
    libusb_device_descriptor Descriptor;
    if(libusb_get_device_descriptor(Devs[i], &Descriptor) < 0) {
      LOGFile::Error("Can not load USB descriptor...\n");
      return false;
    }
    if(Descriptor.idVendor != DEVICEV1_00_VID) continue;
    if(Descriptor.idProduct != DEVICEV1_00_PID) continue;
    if(Descriptor.bcdDevice != DEVICEV1_00_VERSION) continue;
    libusb_device_handle *Dev_Handle = NULL;
    if(libusb_open(Devs[i], &Dev_Handle) != LIBUSB_SUCCESS) {
      LOGFile::Error("Can not open USB device...\n");
    }
    char String[256];
    if(libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iManufacturer, (unsigned char*)String, sizeof(String)) < 0) {
      LOGFile::Error("Can not load USB string...\n");
      libusb_close(Dev_Handle);
      continue;
    }
    if(strcmp(String, DEVICEV1_00_MANSTRING) != 0) continue;
    if(libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iProduct, (unsigned char*)String, sizeof(String)) < 0) {
      LOGFile::Error("Can not load USB string...\n");
      libusb_close(Dev_Handle);
      continue;
    }
    if(strcmp(String, DEVICEV1_00_PRODSTRING) != 0) continue;    
    if(libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iSerialNumber, (unsigned char*)String, sizeof(String)) < 0) {
      LOGFile::Error("Can not load USB string...\n");
      libusb_close(Dev_Handle);
      continue;
    }
    libusb_close(Dev_Handle);
    LOGFile::Info("Adding 'V1.00' device driver instance for USB with serial '%s'\n", String);
    DeviceManager::AddDevice(new DeviceV1_00(String));
  }
  libusb_free_device_list(Devs, Count);
	libusb_exit(Context);
  return true;
}
DeviceV1_00::DeviceV1_00(string serial) {
  pDeviceHandle  = NULL;
  pNewDevice     = NULL;
  pSerial        = serial;
  pDriver        = "V1.00";
  pName          = "Capture device V1.00";
  pVoltageGain   = 0.0;
  pCurrentGain   = 0.0;
  pCurrentInvert = false;
  pVoltageOffset = 0.0;
  pCurrentOffset = 0.0;
  pCurrentInvert = false;
  pCaptureThread = 0;
  pSignalThread  = 0;
  pMutex         = PTHREAD_MUTEX_INITIALIZER;
}
DeviceV1_00::~DeviceV1_00() {
  StopDevice();
  WaitInstances();
}
bool DeviceV1_00::Start(const json &config) {
  pVoltageGain = config["VoltageGain"];
  pVoltageOffset = config["VoltageOffset"];
  pCurrentGain = config["CurrentGain"];
  pCurrentOffset = config["CurrentOffset"];
  pCurrentInvert = config["CurrentInvert"];
  pDelay = deque<float>(2, 0.0);
  pTerminate    = false;
  pDeviceHandle = NULL;
  pNewDevice    = NULL;
  pAnalyzersLock.WriteLock();
  pAnalyzers.push_back(new AnalyzerSoft(pSerial, Analyzer::PHASE1, "/Devices/" + pSerial + "/Analyzer", DEVICEV1_00_SAMPLEFREQ));
  pAnalyzersLock.WriteUnlock();
  if(libusb_init(&pContext) < 0) {
    LOGFile::Error("Can not open libUSB...\n");
    return false;
}
  if(pthread_create(&pCaptureThread, NULL, Capture_Thread, this) != 0) {
    pCaptureThread = 0;
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(pContext);
    return false;
}
  if(libusb_hotplug_register_callback(pContext, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , DEVICEV1_00_VID, DEVICEV1_00_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, this, NULL) < 0) {
    LOGFile::Error("Failed to register libUSB calback...\n");
    pTerminate = true;
    pthread_join(pCaptureThread, NULL);
    pCaptureThread = 0;
    libusb_exit(pContext);
    return false;
}
  pState = RUNNING;
  return true;
}
json DeviceV1_00::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["VoltageGain"] = 0.0;
  ConfigOUT["VoltageOffset"] = 0.0;
  ConfigOUT["CurrentGain"] = 0.0;
  ConfigOUT["CurrentOffset"] = 0.0;
  ConfigOUT["CurrentInvert"] = false;
  ConfigOUT["AutoStart"] = false;
  ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(json::object());
  if(config.is_object()) {
    if(config.contains("VoltageGain") && config["VoltageGain"].is_number() && (config["VoltageGain"] >= -10.0) && (config["VoltageGain"] <= 10.0)) ConfigOUT["VoltageGain"] = config["VoltageGain"];
    if(config.contains("VoltageOffset") && config["VoltageOffset"].is_number() && (config["VoltageOffset"] >= -20.0) && (config["VoltageOffset"] <= 20.0)) ConfigOUT["VoltageOffset"] = config["VoltageOffset"];
    if(config.contains("CurrentGain") && config["CurrentGain"].is_number() && (config["CurrentGain"] >= -10.0) && (config["CurrentGain"] <= 10.0)) ConfigOUT["CurrentGain"] = config["CurrentGain"];
    if(config.contains("CurrentOffset") && config["CurrentOffset"].is_number() && (config["CurrentOffset"] >= -1.0) && (config["CurrentOffset"] <= 1.0)) ConfigOUT["CurrentOffset"] = config["CurrentOffset"];
    if(config.contains("CurrentInvert") && config["CurrentInvert"].is_boolean()) ConfigOUT["CurrentInvert"] = config["CurrentInvert"];    
    if(config.contains("AutoStart") && config["AutoStart"].is_boolean()) ConfigOUT["AutoStart"] = config["AutoStart"];
    if(config.contains("Analyzer") && config["Analyzer"].is_object()) ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(config["Analyzer"]);
}
  return ConfigOUT;
}
bool DeviceV1_00::StartDevice() {
  pthread_mutex_lock(&pMutex);
  if(pState == RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Loading 'V1.01' driver configuration for serial '%s'\n", pSerial.c_str());
  json Config = CheckConfig(ConfigManager::ReadConfig("/Devices/" + pSerial));
  bool AutoStart = Config["AutoStart"];
  ConfigManager::WriteConfig("/Devices/" + pSerial, Config);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    if(AutoStart == false) {
      pthread_mutex_unlock(&pMutex);
      return true;
    }
  }
  LOGFile::Info("Starting 'V1.00' driver for serial '%s'\n", pSerial.c_str());
  bool Result = Start(Config);  
  pthread_mutex_unlock(&pMutex);
  return Result;
}
bool DeviceV1_00::StopDevice() {
  pthread_mutex_lock(&pMutex);
  if(pState != RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Stopping 'V1.00' driver for serial '%s'\n", pSerial.c_str());
  pTerminate = true;
  pthread_join(pCaptureThread, NULL);
  pAnalyzersLock.WriteLock();
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    delete Analyzer;
  }
  pAnalyzersLock.WriteUnlock();
  pState = STOPPED;
  pthread_mutex_unlock(&pMutex);
  return true;
}
void DeviceV1_00::GetDevice(HTTPRequest &req) {
  json Config = ConfigManager::ReadConfig("/Devices/" + pSerial);
  Config.erase("Analyzer");
  Sample_t Avg;
  Avg.Voltage = 0.0;
  Avg.Current = 0.0;
  pAnalyzersLock.ReadLock();
  for(deque<Sample_t>::iterator it = pAverageValues.begin(); it != pAverageValues.end(); it++) {
    Avg.Voltage += it->Voltage;
    Avg.Current += it->Current;
  }
  pAnalyzersLock.ReadUnlock();
  Config["Wave_Average"] = json::object();
  Config["Wave_Average"]["Voltage"] = Avg.Voltage / pAverageValues.size();
  Config["Wave_Average"]["Current"] = Avg.Current / pAverageValues.size();
  req.FromJSON(Config);
}
void DeviceV1_00::SetDevice(HTTPRequest &req) {
  pthread_mutex_lock(&pMutex);
  DeviceState State = pState;
  pthread_mutex_unlock(&pMutex);
  StopDevice();
  json Params = req.GetParams();
  json Config = CheckConfig(Params);
  json Orig = ConfigManager::ReadConfig("/Devices/" + pSerial);
  Config["Analyzer"] = Orig["Analyzer"];
  bool Result = true;
  ConfigManager::WriteConfig("/Devices/" + pSerial, Config);
  pthread_mutex_lock(&pMutex);
  if((pState != RUNNING) && (State == RUNNING)) Result = Start(Config);
  pthread_mutex_unlock(&pMutex);
  json Response;
  Response["Result"] = Result;
  req.FromJSON(Response);
}
void DeviceV1_00::ReadCallback(struct libusb_transfer *transfer) {
  DeviceV1_00 *This = (DeviceV1_00*)transfer->user_data;
  for(int Block = 0; Block < (transfer->actual_length / 64); Block++) {
    int16_t *Pkg   = (int16_t*)&transfer->buffer[64 * Block];
    for(uint8_t Count = 0; Count < 16; Count++) {
      if(Pkg[Count * 2 + 0] == -32768) continue;
      float Voltage = ((Pkg[Count * 2 + 0] * 0.0142446289062) * (1.0 + (This->pVoltageGain / 100.0))) + This->pVoltageOffset;
      float Current = ((Pkg[Count * 2 + 1] * 0.0015625000000) * (1.0 + (This->pCurrentGain / 100.0)) * (This->pCurrentInvert ? -1.0 : 1.0)) + This->pCurrentOffset;
      if(This->BuffersAppend(Voltage, Current) == false) {
        LOGFile::Error("Capture buffer overflow! Posible lost of samples\n");
        break;
      }
    }
  }
  if(This->pTerminate == false) 
    libusb_submit_transfer(transfer);
  else
    libusb_free_transfer(transfer);
}
int DeviceV1_00::PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
  DeviceV1_00 *This = (DeviceV1_00*)user_data;
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (This->pDeviceHandle == NULL)) {
    This->pNewDevice = dev;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    if(This->pDeviceHandle != NULL) {
      libusb_device *CurrentDev = libusb_get_device(This->pDeviceHandle);	
      if(dev == CurrentDev) {
        LOGFile::Info("DeviceV1.00 with serial '%s' disconnected\n", This->pSerial.c_str());
//        This->pTerminateSignal = true;
        pthread_join(This->pSignalThread, NULL);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL;
      }
    }
  }
  return 0;
}
bool DeviceV1_00::BuffersAppend(float voltageSample, float currentSample) {
//  static int File = 0;
//  if(File == 0) File = open("Samples.raw", O_CREAT | O_WRONLY);
//  char Tmp[100];
//  sprintf(Tmp, "%f, %f\n", voltageSample, currentSample);
//  write(File, Tmp, strlen(Tmp));
  
//  if(pBuffer.full()) return false;
  Sample_t Sample = { pDelay.front(), currentSample };
  pBuffer.push_back(Sample);
  pDelay.pop_front();
  pDelay.push_back(voltageSample);
  pAverageValues.push_back(Sample);
  if(pAverageValues.size() > DEVICEV1_00_SAMPLEFREQ) pAverageValues.pop_front();  
  AnalyzerSoft *Analyzer = (AnalyzerSoft*)pAnalyzers[0];
  uint64_t Sampletime = Tools::GetTime();
  if(Analyzer->IsFinished() == false) return true;
  bool Started = false;
  while((pBuffer.empty() == false) && (Started == false)) {
    Sample = pBuffer.front();
    pBuffer.pop_front();
    vector<float> Voltage = {Sample.Voltage};
    vector<float> Current = {Sample.Current};
    if(Analyzer->BufferAppend(Voltage, Current, Sampletime) == true) Started = true;
  }
  return true;
}
void *DeviceV1_00::Capture_Thread(void *args) {
  DeviceV1_00 *This = (DeviceV1_00*)args;
  for(int i = 0; i < DEVICEV1_00_TRANSFERS; i++) This->pTransfers[i].Transfer = libusb_alloc_transfer(0);
  while(!This->pTerminate) {
    struct timeval tv = { 1, 0 };
    int Result = libusb_handle_events_timeout_completed(This->pContext, &tv, NULL);
    if(Result < LIBUSB_SUCCESS) LOGFile::Error("libusb_handle_events_timeout_completed returns code %d\n", Result);
    if(This->pNewDevice != NULL) {
      struct libusb_device_descriptor Descriptor;
      Result = libusb_get_device_descriptor(This->pNewDevice, &Descriptor);
      if(Result < LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_get_device_descriptor return code %d\n", Result);
        This->pNewDevice = NULL;
        continue;
      }
      Result = libusb_open(This->pNewDevice, &This->pDeviceHandle);
      if(Result != LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_open return %d\n", Result);
        This->pDeviceHandle = NULL;
        This->pNewDevice = NULL;
        continue;
      }
      char Serial[256];
      Result = libusb_get_string_descriptor_ascii(This->pDeviceHandle, Descriptor.iSerialNumber, (unsigned char*)Serial, sizeof(Serial));
      if(Result < 0) {
        LOGFile::Error("libusb_get_string_descriptor_ascii returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      }
      if(This->pSerial != Serial) {
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      }        
      Result = libusb_kernel_driver_active(This->pDeviceHandle, 0);
      if(Result < LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_kernel_driver_active returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      } else if(Result == 1) { 
        Result = libusb_detach_kernel_driver(This->pDeviceHandle, 0);
        if(Result < LIBUSB_SUCCESS) {
          LOGFile::Error("libusb_detach_kernel_driver returns %d\n", Result);
          libusb_close(This->pDeviceHandle);
          This->pDeviceHandle = NULL; 
          This->pNewDevice = NULL;
          continue;
        } else {
          LOGFile::Info("Kernel driver dettached for DeviceV1.00 with serial '%s'\n", This->pSerial.c_str());
        }
      }
      Result = libusb_claim_interface(This->pDeviceHandle, 0);
      if(Result != LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_claim_interface returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      } else {
        LOGFile::Info("Claimed interface 0 for DeviceV1.00 with serial %s\n", This->pSerial.c_str());
      }
      for(int i = 0; i < DEVICEV1_00_TRANSFERS; i++) {
        libusb_fill_bulk_transfer(This->pTransfers[i].Transfer, This->pDeviceHandle, DEVICEV1_00_ENDPOINT_IN, This->pTransfers[i].Buffer, sizeof(This->pTransfers[i].Buffer), ReadCallback, This, -1);
        Result |= libusb_submit_transfer(This->pTransfers[i].Transfer);
      }
      if(Result != LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_submit_transfer returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      }
      LOGFile::Info("Capture started on device 'V1.00' with serial '%s'\n", This->pSerial.c_str());
//      This->pTerminateSignal = false;
//      if(pthread_create(&This->pSignalThread, NULL, Signal_Thread, This) != 0) {
//        LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//      }
      This->pNewDevice = NULL;
    }
  }
  return NULL;
}
void *DeviceV1_00::Sig_Thread(void* args) {
/*  DeviceV1 *This = (DeviceV1*)args;
  bool Toogle = false;
  while(!This->pTerminateSignal) {
    AnalyzerInterface::Params_t Params = { AnalyzerInterface::UNINITIALIZED };
    This->pAnalyzer->GetParams(Params, (AnalyzerInterface::ParamsType)(AnalyzerInterface::RMS_V | AnalyzerInterface::RMS_V));
    uint16_t Current = 0;
    uint16_t Voltage = 1 << 6;
    uint8_t  Beep    = 0;
    if(Params.RMS_I[0] > (( 1.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 0;
    if(Params.RMS_I[0] > (( 2.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 1;
    if(Params.RMS_I[0] > (( 3.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 2;
    if(Params.RMS_I[0] > (( 4.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 3;
    if(Params.RMS_I[0] > (( 5.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 4;
    if(Params.RMS_I[0] > (( 6.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 5;
    if(Params.RMS_I[0] > (( 7.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 6;
    if(Params.RMS_I[0] > (( 8.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 7;
    if(Params.RMS_I[0] > (( 9.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 8;
    if(Params.RMS_I[0] > ((10.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 9;
    if(Params.RMS_I[0] > ((11.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 10;
    if(Params.RMS_I[0] > ((12.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 11;
    if(Params.RMS_I[0] > ((13.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 12;
    if(Params.RMS_I[0] > (0.85 * sqrt(This->pAnalyzer->GetMaxCurrent()))) Beep = 10;
    if(Params.RMS_V[0] < ((1.0 - (6.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 0;
    if(Params.RMS_V[0] < ((1.0 - (5.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 1;
    if(Params.RMS_V[0] < ((1.0 - (4.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 2;
    if(Params.RMS_V[0] < ((1.0 - (6.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 3;
    if(Params.RMS_V[0] < ((1.0 - (2.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 4;
    if(Params.RMS_V[0] < ((1.0 - (1.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 5;
    if(Params.RMS_V[0] > ((1.0 + (1.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 7;
    if(Params.RMS_V[0] > ((1.0 + (2.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 8;
    if(Params.RMS_V[0] > ((1.0 + (3.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 9;
    if(Params.RMS_V[0] > ((1.0 + (4.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 10;
    if(Params.RMS_V[0] > ((1.0 + (5.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 11;
    if(Params.RMS_V[0] > ((1.0 + (6.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 12;
    if(Params.Flag == true) {
      Beep = 50;
      if(Toogle == true) Voltage = 0x0000;
      Toogle = !Toogle;  
    }
    uint8_t Write[5] = {(uint8_t)(Current & 0xFF), (uint8_t)(Current >> 8), (uint8_t)(Voltage & 0xFF), (uint8_t)(Voltage >> 8), Beep};
    libusb_control_transfer(This->pDeviceHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0xFF, 0x0000, 0x0000, Write, sizeof(Write), 0);
    uint8_t Read[2];
    libusb_control_transfer(This->pDeviceHandle, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0xFE, 0x0000, 0x0000, Read, sizeof(Read), 0);
    Tools::pBatteryLevel = Read[1];
    Tools::pBatteryCharging = (Read[0] & 0x02);
    Tools::pExternalPower = (Read[0] & 0x01);
    fflush(stdout);
    usleep(100000);
    //Envio transferencia de control
    //Leo datos, si hay algun boton pulsado llamo a los scripts!
  }
 */
  return NULL;
}
//uint32_t CaptureMono::SamplesCopy(float **dest, uint32_t offset, uint32_t len) {
//  for(uint i = 0; i < len; i++) {
//    uint32_t Pos = ((ReadPos + CAPTUREMONO_BUFFERSIZE) - offset + i) % CAPTUREMONO_BUFFERSIZE;
//    dest[i][0] = Buffer[Pos].Voltage;
//  }
//  return len;
//}
//void Adquisition_SetLEDs(float voltage, float current) {
//int Error = libusb_control_transfer(This->pDeviceHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0x01, 0x0000, 0x0000, NULL, 0, 0);
//  uint8_t Val = 0;
//  int Voltage = round(((voltage / EVENT_VOLTAGE) - 1.0) * 50.0 + 4.0);
//  if(Voltage < 0) Voltage = 0;
//  if(Voltage > 8) Voltage = 8;
//  int Current = round((current / MAX_CURRENT) * 100.0 / 9.0);
//  if(Current < 0) Current = 0;
//  if(Current > 8) Current = 8;
//  Val = (Current << 4) + Voltage;
//}
//======================================================================================================================