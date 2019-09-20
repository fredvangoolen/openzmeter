#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "STM32F0xx.h"
#include "USB.h"

// Device descriptor ---------------------------------------------------------------------------------------------------
const uint8_t USB_Descriptor[] = {
  0x12,                                   /* bLength */
  USB_DTYPE_DEVICE,                       /* bDescriptorType */
  0x00, 0x02,                             /* bcdUSB = 2.00 */
  0xFF,                                   /* bDeviceClass: vendor_defined */
  0x00,                                   /* bDeviceSubClass */
  0x00,                                   /* bDeviceProtocol */
  0x08,                                   /* bMaxPacketSize0 */
  0x83, 0x04,                             /* idVendor */
  0x70, 0x72,                             /* idProduct */
  0x02, 0x02,                             /* bcdDevice = 2.02 */
  1,                                      /* Index of string descriptor describing manufacturer */
  2,                                      /* Index of string descriptor describing product */
  3,                                      /* Index of string descriptor describing the device's serial number */
  0x01                                    /* bNumConfigurations */
};

const uint8_t StringLangID[] = { 
  0x04,                                   /* bLength */
  USB_DTYPE_STRING,                       /* bDescriptorType */
  0x0A, 0x04                              /* LangID = 0x040A: Spanish */
};
const uint8_t StringVendor[] = {
  0x0A,                                   /* bLength */
  USB_DTYPE_STRING,                       /* bDescriptorType*/
  'z', 0, 'R', 0, 'e', 0, 'd', 0
};
const uint8_t StringProduct[] = {
  0x38,                                   /* bLength */
  USB_DTYPE_STRING,                       /* bDescriptorType */
  'o', 0, 'p', 0, 'e', 0, 'n', 0, 'Z', 0, 'm', 0, 'e', 0, 't', 0, 
  'e', 0, 'r', 0, ' ', 0, '-', 0, ' ', 0, 'C', 0, 'a', 0, 'p', 0,
  't', 0, 'u', 0, 'r', 0, 'e', 0, ' ', 0, 'd', 0, 'e', 0, 'v', 0,
  'i', 0, 'c', 0, 'e', 0 
};
//uint8_t StringSerial[50];
//const uint8_t *USB_Strings[] = { &StringLangID, &StringVendor, &StringProduct};
//----------------------------------------------------------------------------------------------------------------------


#define CDC_EP0_SIZE    0x08
//#define CDC_RXD_EP      0x01
#define CDC_TXD_EP      0x81
#define CDC_DATA_SZ     0x40
//#define CDC_NTF_EP      0x82
//#define CDC_NTF_SZ      0x08
//#define HID_RIN_EP      0x83
//#define HID_RIN_SZ      0x10

#define CDC_LOOPBACK


volatile usbd_device udev;
uint32_t	ubuf[0x20] __attribute__ ((aligned (4)));
//uint8_t     fifo[0x200];
uint32_t    fpos = 0;


/*

USB_Response cdc_getdesc (usbd_ctlreq *req, void **address, uint16_t *length) {
  const uint8_t dtype = req->wValue >> 8;
  const uint8_t dnumber = req->wValue & 0xFF;
  const uint8_t *desc;
  uint16_t len = 0;
  switch (dtype) {
    case USB_DTYPE_CONFIGURATION :
      desc = USB_ConfigDescriptor;
      len = sizeof(USB_ConfigDescriptor);
      break;
//    case USB_DTYPE_STRING :
//      if(dnumber < 3) {
//        desc = USB_Strings[dnumber];
//      } else {
//        return USB_FAIL;
//      }
//      break;
    default:
      return USB_FAIL;
  }
  if(len == 0) {
    len = desc[0];
  }
  *address = (void*)desc;
  *length = len;
  return USB_ACK;
};
*/

USB_Response cdc_control(usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback) {
    if (((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) == (USB_REQ_INTERFACE | USB_REQ_CLASS)
        && req->wIndex == 0 ) {
        switch (req->bRequest) {
        case USB_CDC_SET_CONTROL_LINE_STATE:
            return USB_ACK;
        case USB_CDC_SET_LINE_CODING:
            //memcpy( req->data, &cdc_line, sizeof(cdc_line));
            return USB_ACK;
//        case USB_CDC_GET_LINE_CODING:
//            dev->status.data_ptr = &cdc_line;
//            dev->status.data_count = sizeof(cdc_line);
//            return usbd_ack;
        default:
            return USB_FAIL;
        }
    }
    return USB_FAIL;
}







/* CDC loop callback. Both for the Data IN and Data OUT endpoint */
//static void cdc_loopback(usbd_device *dev, uint8_t event, uint8_t ep) {
//    int _t;
//    switch (event) {
//    case usbd_evt_eptx:
//        _t = ep_write(CDC_TXD_EP, &fifo[0], (fpos < CDC_DATA_SZ) ? fpos : CDC_DATA_SZ);
//        if (_t > 0) {
//            memmove(&fifo[0], &fifo[_t], fpos - _t);
//            fpos -= _t;
//        }
//    case usbd_evt_eprx:
//        if (fpos < (sizeof(fifo) - CDC_DATA_SZ)) {
//            _t = ep_read(CDC_RXD_EP, &fifo[fpos], CDC_DATA_SZ);
//            if (_t > 0) {
//                fpos += _t;
//            }
//        }
//    default:
//        break;
//    }
//}
USB_Response cdc_setconf (usbd_device *dev, uint8_t cfg) {
  switch (cfg) {
    case 0:
        /* deconfiguring device */
//        USB_DeconfigEP(CDC_NTF_EP);
        USB_DeconfigEP(CDC_TXD_EP);
//        USB_DeconfigEP(CDC_RXD_EP);
//        usbd_reg_endpoint(dev, CDC_RXD_EP, 0);
        usbd_reg_endpoint(dev, CDC_TXD_EP, 0);
        return USB_ACK;
    case 1:
        /* configuring device */
//        USB_ConfigEP( CDC_RXD_EP, USB_EPTYPE_BULK /*| USB_EPTYPE_DBLBUF*/, CDC_DATA_SZ);
        USB_ConfigEP( CDC_TXD_EP, USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF, CDC_DATA_SZ);
//        USB_ConfigEP( CDC_NTF_EP, USB_EPTYPE_INTERRUPT, CDC_NTF_SZ);
//#if defined(CDC_LOOPBACK)
//        usbd_reg_endpoint(dev, CDC_RXD_EP, cdc_loopback);
//        usbd_reg_endpoint(dev, CDC_TXD_EP, cdc_loopback);
//#else
//        usbd_reg_endpoint(dev, CDC_RXD_EP, cdc_rxonly);
        usbd_reg_endpoint(dev, CDC_TXD_EP, cdc_txonly);
//#endif
        ep_write(CDC_TXD_EP, 0, 0);
        return USB_ACK;
    default:
        return USB_FAIL;
    }
}
  void IntToUnicode(uint32_t value, uint8_t *pbuf) {
  for(uint8_t i = 0; i < 8; i++) {
    *(pbuf++) = (value & 0x0F) + (((value & 0x0F) < 0x0A) ? '0' : 'A' - 10);
    *(pbuf++) = 0;
    value = value >> 4;
  }
}

//void cdc_init_usbd(void) {
//  uint8_t *dsc = StringSerial;
//  dsc[0] = 50;
//  dsc[1] = USB_DTYPE_STRING;
//  IntToUnicode(*(__IO uint32_t*)(0x1FFFF7AC), &dsc[2]);
//  IntToUnicode(*(__IO uint32_t*)(0x1FFFF7B0), &dsc[18]);
//  IntToUnicode(*(__IO uint32_t*)(0x1FFFF7B4), &dsc[34]);
  
  
//    usbd_init(&udev, CDC_EP0_SIZE, ubuf, sizeof(ubuf));
//}

  void usbd_init(/*usbd_device *dev,*/
                             /*const uint8_t ep0size, uint32_t *buffer, const uint16_t bsize*/) {
//    dev->driver = drv;
    udev.status.ep0size = CDC_EP0_SIZE;
    udev.status.data_ptr = ubuf;
    udev.status.data_buf = ubuf;
    udev.status.data_maxsize = sizeof(ubuf) - __builtin_offsetof(usbd_ctlreq, data);
}