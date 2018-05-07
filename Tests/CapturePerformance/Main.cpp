#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <libusb-1.0/libusb.h>
//Constantes del puerto USB ----------------------------------------------------
#define USB_VENDOR_ID      0x0483                      /* USB vendor ID used by the device 0x0483 is STMs ID */
#define USB_PRODUCT_ID     0x7270                      /* USB product ID used by the device */
#define USB_ENDPOINT_IN    (LIBUSB_ENDPOINT_IN  | 1)   /* endpoint address */
#define LEN_IN_BUFFER      8192
#define TRANSFERS          16
#define USB_SERIAL         "330053007075435593532302"
//Estructuras de datos del USB -------------------------------------------------
struct libusb_device        *NewDeviceArrive = NULL;
struct libusb_device_handle *Device          = NULL;
struct libusb_transfer      *Transfer[TRANSFERS];
static uint8_t Buffer[TRANSFERS][LEN_IN_BUFFER];
enum Sync_t {RESET, WAIT, SYNC};
//Temporizacion y control del programa ---------------------------------------------------------------------------------
bool     Terminate = false;
Sync_t   Sync      = RESET;
uint64_t Time      = 0;
uint32_t Packets   = 0;
uint32_t Bytes     = 0;
uint8_t  Sequence  = 0;
//----------------------------------------------------------------------------------------------------------------------
void SendSetupCommand(uint8_t channels, uint8_t mode) {
  Sequence = 0;
  printf("Enviando 'Iniciar captura'\n");
  int Error = libusb_control_transfer(Device, 0x21, 0x00, (channels << 8) | mode, 0, NULL, 0, 0);
  if(Error < 0) {
    printf("Error in libusb_control_transfer (%d)\n", Error);
  }
}
uint64_t GetTime() {
  struct timespec  Tmp;
  clock_gettime(CLOCK_REALTIME, &Tmp);
  return Tmp.tv_sec * 1000000L + Tmp.tv_nsec / 1000;
}
int HotPlug_Callback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (Device == NULL)) {
    NewDeviceArrive = dev;
    return 0;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    libusb_device *CurrentDev = libusb_get_device(Device);
    if(dev == CurrentDev) {
      printf("Device left!!\n");
      Sync = RESET;
      for(int i = 0; i < TRANSFERS; i++) {
        libusb_free_transfer(Transfer[i]);
        Transfer[i] = NULL;
      }
      libusb_close(Device);
      Device = NULL;
    }
  } else {
    printf("Unhandled event %d\n", event);
  }
  return 0;
}
void In_Callback(struct libusb_transfer *transfer) {
  if(transfer->status == LIBUSB_TRANSFER_COMPLETED) {
    uint8_t *Ptr = transfer->buffer;
    for(int i = 0; i < transfer->actual_length / 64; i++) {
      uint8_t Seq = Ptr[0];
      Sequence++;
      if((Sync == SYNC) && (Seq != Sequence)) {
        printf("Fallo de sincronizacion (Recibido %d - Esperado %d)!!\n", Seq, Sequence);
        Sync = WAIT;          
      }
      if(Sync == WAIT) {
        Sequence = Seq;
        printf("Sincronizando -> %d\n", Sequence);
        Sync = SYNC;
      }
      if(Sync == RESET) {
        Ptr = &Ptr[64];
        printf("Omitiendo datos no esperados!!\n");
        continue;
      }
      Packets++;
      Bytes += Ptr[1] * 2;
      if(Packets == 10000) {
        uint64_t Now = GetTime();
        double Diff = Now - Time;
        printf("Received %u transfers and %u bytes in %.1f ms, %.1f kSamples/s\n", Packets, Bytes, Diff / 1000.0, (Bytes / 2000.0) / (Diff / 1000000.0));
        fflush(stdout);
        Time = GetTime();
        Bytes = 0;
        Packets = 0;
      }
      Ptr = &Ptr[64];
    }
  }
  libusb_submit_transfer(transfer);
}
bool ConnectToDevice() {
  int Error =  libusb_kernel_driver_active(Device, 0);
  if(Error < 0) {
    printf("libusb_kernel_driver_active error (%d)", Error);
    return false;
  } else if(Error == 1) { 
    int Error = libusb_detach_kernel_driver(Device, 0);
    if(Error < 0) {
      printf("libusb_detach_kernel_driver error %d\n", Error);
      return false;
    } else {
      printf("Kernel driver dettached\n");
    }
  }
  Error = libusb_claim_interface(Device, 0);
  if(Error < 0) {
    printf("usb_claim_interface error %d\n", Error);
    return false;
  } else {
    printf("Claimed interface\n");
  }
  for(int i = 0; i < TRANSFERS; i++) {
    if(Transfer[i] == NULL) Transfer[i] = libusb_alloc_transfer(0);
    libusb_fill_bulk_transfer(Transfer[i], Device, USB_ENDPOINT_IN, Buffer[i], LEN_IN_BUFFER, In_Callback, NULL, 0);
    Error = libusb_submit_transfer(Transfer[i]);
    if(Error < 0) {
      printf("Error in submit_transfer (%d)\n", Error);
      return false;
    }
  }
  printf("Entering loop to process callbacks...\n");
  Time = GetTime();
  return true;
}
int main(void) {
  uint64_t Osc = 72000000;
  int Order = 0;
  for(int Div = 1; Div < Osc / 1; Div++) {
    if((Osc % Div) == 0) {
      int SampleRate = Osc / Div;
      if(SampleRate > 480000) continue;
      if((SampleRate % 64) != 0) continue;
      if((SampleRate >= 5000)) {
        printf("Order %d (%d) -> SampleRate %d (4x -> %f) (16x -> %f) (64x -> %f)\n", Order, Div, SampleRate, SampleRate / 4.0, SampleRate / 16.0, SampleRate / 64.0);
        Order++;
      }
    }
  }

  //return 0;
  
  
  
  
  
  int Error = libusb_init(NULL);
  if(Error < 0) {
    printf("Failed to initialise libusb\n");
    return 1;
  }
  for(int i = 0; i < TRANSFERS; i++) Transfer[i] = NULL;
  libusb_hotplug_callback_handle handle;
  Error = libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , USB_VENDOR_ID, USB_PRODUCT_ID, LIBUSB_HOTPLUG_MATCH_ANY, HotPlug_Callback, NULL, &handle);
  if(Error < 0) {
    printf("Failed to register hotplug callback");
    return 1;
  }
  while(!Terminate) {
    struct timeval Timeout = { 1, 0 };
    int Error = libusb_handle_events_timeout_completed(NULL, &Timeout, NULL);
    if((Device != NULL) && (Sync == RESET)) {
      uint8_t Channels = 0x0F;
      uint8_t Mode = 0x1F;
      Sync = WAIT;
      SendSetupCommand(Channels, Mode);
    }
    if(Error < 0) { // negative values are errors
      printf("Error in libusb_handle_events_timeout_completed (%d)\n", Error);
      return 0;
    }
    if(NewDeviceArrive) {
      struct libusb_device_descriptor desc;
      Error = libusb_get_device_descriptor(NewDeviceArrive, &desc);
      if(Error < 0) {
        printf("libusb_get_device_descriptor fail %d\n", Error);
        return 0;
      }
      Error = libusb_open(NewDeviceArrive, &Device);
      if(LIBUSB_SUCCESS != Error) {
        printf("Could not open USB device (%d)\n", Error);
      }
      char ReadSerial[256];
      Error = libusb_get_string_descriptor_ascii(Device, desc.iSerialNumber, (unsigned char*)ReadSerial, sizeof(ReadSerial));
      if(Error < 0) {
        printf("libusb_get_string_descriptor_ascii fail %d\n", Error);
        return 1;
      }
      if(strcmp(ReadSerial, USB_SERIAL) == 0) {
        printf("New device arrive with serial %s\n", ReadSerial);
        ConnectToDevice();
      } else {
        Device = NULL;
      }
      NewDeviceArrive = NULL;
    }
  }
  return 0;
}

