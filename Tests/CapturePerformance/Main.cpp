#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>

#define USB_VENDOR_ID      0x0483      /* USB vendor ID used by the device 0x0483 is STMs ID */
#define USB_PRODUCT_ID     0x7270      /* USB product ID used by the device */
#define USB_ENDPOINT_IN    (LIBUSB_ENDPOINT_IN  | 1)   /* endpoint address */
#define LEN_IN_BUFFER      8192

//#define USB_ENDPOINT_OUT	(LIBUSB_ENDPOINT_OUT | 1)   /* endpoint address */

struct libusb_device_handle *devh   = NULL;
struct libusb_transfer *TransferOUT = NULL;
struct libusb_transfer *TransferIN  = NULL;
static uint8_t BufferIN[LEN_IN_BUFFER];
bool Terminate = 0;
struct libusb_device *NewDeviceArrive = NULL;
char Serial[] = "25FFB6602756158491734076";
// Function Prototypes:
//void sighandler(int signum);
//void print_libusb_transfer(struct libusb_transfer *p_t);

uint32_t benchPackets = 1;
uint32_t benchBytes = 0;
struct timespec t1, t2;


bool ConnectToDevice();
void cb_out(struct libusb_transfer *transfer) {
  fprintf(stderr, "cb_out: status =%d, actual_length=%d\n", transfer->status, transfer->actual_length);
}
int hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
//  static libusb_device_handle *handle = NULL;
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (devh == NULL)) {
    NewDeviceArrive = dev;
    return 0;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    libusb_device *CurrentDev = libusb_get_device(devh);	
    if(dev == CurrentDev) {
        printf("Device left!!\n");
//      libusb_cancel_transfer(TransferIN);
      libusb_close(devh);
//      libusb_free_transfer(TransferIN);
      devh = NULL;
      TransferIN = NULL;
    }
  } else {
    printf("Unhandled event %d\n", event);
  }
  return 0;
}
void cb_in(struct libusb_transfer *transfer) {
  clock_gettime(CLOCK_REALTIME, &t2);
  if(transfer->status == LIBUSB_TRANSFER_COMPLETED) {
    //Copy to my own space
  } 
  libusb_submit_transfer(transfer);
  if(transfer->status != LIBUSB_TRANSFER_COMPLETED) return;
  benchBytes += transfer->actual_length;
  if(benchPackets % 100 == 0){
    uint32_t diff = (t2.tv_sec - t1.tv_sec)*1000000000L + (t2.tv_nsec - t1.tv_nsec);
    t1.tv_sec = t2.tv_sec;
    t1.tv_nsec = t2.tv_nsec;
    printf("received %u transfers and %u bytes in %u us, %8.1f kB/s\n", benchPackets, benchBytes, diff / 1000, benchBytes * 1000000.0 / diff);
    fflush(stdout);
    benchPackets = 0;
    benchBytes = 0;
  }
  benchPackets++;
}
bool ConnectToDevice() {
  int r =  libusb_kernel_driver_active(devh, 0);
  if(r < 0) {
    printf("libusb_kernel_driver_active error (%d)", r);
    return false;
  } else if( r == 1) { 
    int r = libusb_detach_kernel_driver(devh, 0);
    if(r < 0) {
      printf("libusb_detach_kernel_driver error %d\n", r);
      return false;
    } else {
      printf("Kernel driver dettached\n");
    }
  }
  r = libusb_claim_interface(devh, 0);
  if(r < 0) {
    printf("usb_claim_interface error %d\n", r);
    return false;
  } else {
    printf("Claimed interface\n");
  }
  if(TransferIN == NULL) TransferIN = libusb_alloc_transfer(0);
  libusb_fill_bulk_transfer(TransferIN, devh, USB_ENDPOINT_IN, BufferIN, LEN_IN_BUFFER, cb_in, NULL, 0);
  clock_gettime(CLOCK_REALTIME, &t1);
  r = libusb_submit_transfer(TransferIN);
  printf("Entering loop to process callbacks...\n");
  
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
  int r = libusb_init(NULL);
  if (r < 0) {
    fprintf(stderr, "Failed to initialise libusb\n");
    return 1;
  }
  libusb_hotplug_callback_handle handle;
  int rc = libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , USB_VENDOR_ID, USB_PRODUCT_ID, LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, NULL, &handle);
//  devh = libusb_open_device_with_vid_pid(NULL, USB_VENDOR_ID, USB_PRODUCT_ID);
//  if(!devh) {
//    printf("Device not connected, waiting....\n");
//  } else {
//    ConnectToDevice();
//  }
  while(true) {
    struct timeval tv = { 1, 0 };
    int Result = 0;
    int r = libusb_handle_events_timeout_completed(NULL, &tv, &Result);
    if(NewDeviceArrive) {
      struct libusb_device_descriptor desc;
      int r = libusb_get_device_descriptor(NewDeviceArrive, &desc);
      if(r < 0) {
        printf("libusb_get_device_descriptor fail %d\n", r);
        return 0;
      }
      r = libusb_open(NewDeviceArrive, &devh);
      if (LIBUSB_SUCCESS != r) {
        printf("Could not open USB device\n");
      }
      char ReadSerial[256];
      r = libusb_get_string_descriptor_ascii(devh, desc.iSerialNumber, (unsigned char*)ReadSerial, sizeof(ReadSerial));
      if(r < 0) {
        printf("libusb_get_string_descriptor_ascii fail %d\n", r);
        return 1;
      }
      if(strcmp(ReadSerial, Serial) == 0) {
        printf("New device arrive with serial %s\n", ReadSerial);
        ConnectToDevice();
      } else {
        devh = NULL;
      }
      NewDeviceArrive = NULL;
    }
      
//        struct timeval tv = {0, 1000};
//   r = libusb_handle_events_timeout_completed(ctx, &tv, );
//        r = libusb_handle_events(NULL);
 //     r = libusb_handle_events_completed(ctx, NULL);
//        printf("%d", Result);
        fflush(stdout);
      if (r < 0) { // negative values are errors
//        exitflag = out_deinit;
        break;
      }
    }
  

  // If these transfers did not complete then we cancel them.
  // Unsure if this is correct...
//  if (transfer_out) {
//    r = libusb_cancel_transfer(transfer_out);
//    if (0 == r) {
//      printf("transfer_out successfully cancelled\n");
//    }
//    if (r < 0) {
//      exitflag = out_deinit;
//    }
//  }
//  if (transfer_in) {
//    r = libusb_cancel_transfer(transfer_in);
//    if (0 == r) {
//      printf("transfer_in successfully cancelled\n");
//    }
//    if (r < 0) {
//      exitflag = out_deinit;
//    }
//  }

//  switch (exitflag) {
//    case out_deinit:
//      printf("at out_deinit\n");
////      libusb_free_transfer(transfer_out);
////      libusb_free_transfer(transfer_in);
//
//    case out_release:
//      libusb_release_interface(devh, 0);
//    case out:
//      libusb_close(devh);
//      libusb_exit(NULL);
//  }
  return 0;
}

