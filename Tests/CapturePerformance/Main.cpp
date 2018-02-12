#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>
//Constantes del puerto USB ----------------------------------------------------
#define USB_VENDOR_ID      0x0483                      /* USB vendor ID used by the device 0x0483 is STMs ID */
#define USB_PRODUCT_ID     0x7270                      /* USB product ID used by the device */
#define USB_ENDPOINT_IN    (LIBUSB_ENDPOINT_IN  | 1)   /* endpoint address */
#define LEN_IN_BUFFER      8192
#define USB_SERIAL         "25FFB6602756158491734076"
//Estructuras de datos del USB -------------------------------------------------
struct libusb_device        *NewDeviceArrive = NULL;
struct libusb_device_handle *Device          = NULL;
struct libusb_transfer      *TransferA       = NULL;
struct libusb_transfer      *TransferB       = NULL;
static uint8_t BufferA[LEN_IN_BUFFER];
static uint8_t BufferB[LEN_IN_BUFFER];
//Temporizacion y control del programa -----------------------------------------
bool     Terminate    = false;
bool     Open         = false;
uint64_t Time         = 0;
uint32_t benchPackets = 0;
uint32_t benchBytes   = 0;


//
// Function Prototypes:
//void sighandler(int signum);
//void print_libusb_transfer(struct libusb_transfer *p_t);


uint64_t Gettime() {
  struct timespec  Time;
  clock_gettime(CLOCK_REALTIME, &Time);
  return Time.tv_sec * 1000000000L + Time.tv_nsec;
}
int HotPlug_Callback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (Device == NULL)) {
    NewDeviceArrive = dev;
    return 0;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    libusb_device *CurrentDev = libusb_get_device(Device);	
    if(dev == CurrentDev) {
      printf("Device left!!\n");
      libusb_free_transfer(TransferA);
      TransferA = NULL;
      libusb_free_transfer(TransferB);
      TransferB = NULL;
      libusb_close(Device);
      Device = NULL;
    }
  } else {
    printf("Unhandled event %d\n", event);
  }
  return 0;
}
void In_Callback(struct libusb_transfer *transfer) {
//  clock_gettime(CLOCK_REALTIME, &Time2);
  if(transfer->status == LIBUSB_TRANSFER_COMPLETED) {
    //Copy to my own space
  } 
  libusb_submit_transfer(transfer);
 // if(transfer->status != LIBUSB_TRANSFER_COMPLETED) return;
  benchBytes += transfer->actual_length;
  if(benchPackets % 100 == 0){
//    uint32_t diff = (t2.tv_sec - t1.tv_sec)*1000000000L + (t2.tv_nsec - t1.tv_nsec);
//    t1.tv_sec = t2.tv_sec;
//    t1.tv_nsec = t2.tv_nsec;
//    printf("received %u transfers and %u bytes in %u us, %8.1f kB/s\n", benchPackets, benchBytes, diff / 1000, benchBytes * 1000000.0 / diff);
//    fflush(stdout);
    benchPackets = 0;
    benchBytes = 0;
  }
  benchPackets++;
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
  if(TransferA == NULL) TransferA = libusb_alloc_transfer(0);
  if(TransferB == NULL) TransferB = libusb_alloc_transfer(0);
  libusb_fill_bulk_transfer(TransferA, Device, USB_ENDPOINT_IN, BufferA, LEN_IN_BUFFER, In_Callback, NULL, 0);
  Error = libusb_submit_transfer(TransferA);
  if(Error < 0) {
    printf("Error in submit_transfer (%d)\n", Error);
    return false;
  }
  libusb_fill_bulk_transfer(TransferB, Device, USB_ENDPOINT_IN, BufferB, LEN_IN_BUFFER, In_Callback, NULL, 0);
  Error = libusb_submit_transfer(TransferB);
  if(Error < 0) {
    printf("Error in submit_transfer (%d)\n", Error);
    return false;
  }
  printf("Entering loop to process callbacks...\n");
  
  //  clock_gettime(CLOCK_REALTIME, &t1);
  
  
    // Define signal handler to catch system generated signals
    // (If user hits CTRL+C, this will deal with it.)
//    sigact.sa_handler = sighandler; // sighandler is defined below. It just sets do_exit.
//    sigemptyset(&sigact.sa_mask);
//    sigact.sa_flags = 0;
//    sigaction(SIGINT, &sigact, NULL);
//    sigaction(SIGTERM, &sigact, NULL);
//    sigaction(SIGQUIT, &sigact, NULL);
  return true;
}
int main(void) {
  int Osc = 72000000;
  int Order = 0;
  for(int Div = 1; Div < Osc / 4; Div++) {
    if(((Osc % Div) == 0) && ((Osc % (Div * 4)) == 0)) {
      int SampleRate = Osc / Div;
      int Freq = SampleRate / 4;
      if(SampleRate > 212000) continue;
      if((Freq >= 5000) && (Freq <= 120000) && (((Freq % 100) == 0)  || ((Freq % 120) == 0))) {
        printf("Order %d -> Freq %d, SampleRate %d, Oversample (4x)\n", Order, Freq, SampleRate);
        Order++;
      }
    }
  }
  for(int Div = 1; Div < Osc / 16; Div++) {
    if(((Osc % Div) == 0) && ((Osc % (Div * 16)) == 0)) {
      int SampleRate = Osc / Div;
      int Freq = SampleRate / 16;
      if(SampleRate > 212000) continue;
      if((Freq >= 5000) && (Freq <= 120000) && (((Freq % 100) == 0)  || ((Freq % 120) == 0))) {
        printf("Order %d -> Freq %d, SampleRate %d, Oversample (16x)\n", Order, Freq, SampleRate);
        Order++;
      }
    }
  }
  return 0;
  
  
  
  
  
  int Error = libusb_init(NULL);
  if(Error < 0) {
    printf("Failed to initialise libusb\n");
    return 1;
  }
  libusb_hotplug_callback_handle handle;
  Error = libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , USB_VENDOR_ID, USB_PRODUCT_ID, LIBUSB_HOTPLUG_MATCH_ANY, HotPlug_Callback, NULL, &handle);
  if(Error < 0) {
    printf("Failed to register hotplug callback");
    return 1;
  }
  while(!Terminate) {
    struct timeval Timeout = { 1, 0 };
    if((Device != NULL) && (Open == false)) {
      printf("Enviando 'Iniciar captura'\n");
      int Error = libusb_control_transfer(Device, 0x21, 0x00, 0, 0, NULL, 0, 0);
      if(Error < 0) {
        printf("Error in libusb_control_transfer (%d)\n", Error);
        return 1;
      }
      Open = true;
    }
    int Error = libusb_handle_events_timeout_completed(NULL, &Timeout, NULL);
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
    
      
//        struct timeval tv = {0, 1000};
//   r = libusb_handle_events_timeout_completed(ctx, &tv, );
//        r = libusb_handle_events(NULL);
 //     r = libusb_handle_events_completed(ctx, NULL);
//        printf("%d", Result);
        fflush(stdout);

    }

  return 0;
}

