/* This file is the part of the Lightweight USB device Stack for STM32 microcontrollers
 *
 * Copyright ©2016 Dmitry Filimonchuk <dmitrystu[at]gmail[dot]com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#include <stdint.h>
//#include <stdbool.h>
#include "STM32F0xx.h"
#include "USB.h"


const uint8_t USB_ConfigDescriptor[] = {
  0x09,                                   /* bLength: Configuation Descriptor size */
  USB_DTYPE_CONFIGURATION,                /* bDescriptorType: Configuration */
  0x19, 0x00,                             /* wTotalLength:no of returned bytes */
  0x01,                                   /* bNumInterfaces: 1 interface */
  0x01,                                   /* bConfigurationValue: Configuration value */
  0x00,                                   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,                                   /* bmAttributes: self powered */
  0x00,                                   /* MaxPower 0 mA */

  0x09,                                   /* bLength: Interface Descriptor size */
  USB_DTYPE_INTERFACE,                    /* bDescriptorType: Interface */
  0x00,                                   /* bInterfaceNumber: Number of Interface */
  0x00,                                   /* bAlternateSetting: Alternate setting */
  0x01,                                   /* bNumEndpoints: One endpoints used */
  0xFF,                                   /* bInterfaceClass: Communication Interface Class */
  0xFF,                                   /* bInterfaceSubClass: Abstract Control Model */
  0xFF,                                   /* bInterfaceProtocol: No protocol */
  0x00,                                   /* iInterface: */
  
  0x07,                                   /* bLength: Endpoint Descriptor size */
  USB_DTYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  0x81,                                   /* bEndpointAddress: (IN1) */
  0x02,                                   /* bmAttributes: Bulk */
  0x40, 0x00,                             /* wMaxPacketSize: */
  0x00,                                   /* bInterval: ignore for Bulk transfer */
};


void usbd_process_ep0 (usbd_device *dev, uint8_t event, uint8_t ep);



/** \brief Callback that sets USB device address
 * \param dev pointer to usb device
 * \param req pointer to usb control request data
 * \return none
 */
void USB_SetAddress(usbd_device *dev, usbd_ctlreq *req) {
  USB->DADDR = USB_DADDR_EF | req->wValue;
  dev->status.device_state = (req->wValue) ? USB_STATE_ADDRESSED : USB_STATE_DEFAULT;
}

/** \brief Control transfer completion callback processing
 * \param dev pointer to the usb device
 * \return none
 */
static void usbd_process_callback (usbd_device *dev) {
    if (dev->complete_callback) {
        dev->complete_callback(dev, dev->status.data_buf);
        dev->complete_callback = 0;
    }
}

/** \brief SET_CONFIG request processing
 * \param dev usbd_device
 * \param config config number from request
 * \return usbd_ack if success
 */
USB_Response cdc_setconf (usbd_device *dev, uint8_t cfg);
//USB_Response usbd_configure(usbd_device *dev, uint8_t config) {
//        if (cdc_setconf(dev, config) == USB_ACK) {
//            dev->status.device_cfg = config;
//            dev->status.device_state = (config) ? USB_STATE_CONFIGURED : USB_STATE_ADDRESSED;
//            return USB_ACK;
//        }
//    return USB_FAIL;
//}


/** \brief Standard control request processing for device
 * \param dev pointer to usb device
 * \param req pointer to control request
 * \return TRUE if request is handled
 */
//USB_Response cdc_getdesc (usbd_ctlreq *req, void **address, uint16_t *length);
USB_Response usbd_process_devrq(usbd_ctlreq *req) {
  switch(req->bRequest) {
    case USB_STD_CLEAR_FEATURE :
        /* not yet supported */
        break;
    case USB_STD_GET_CONFIG :
        req->data[0] = udev.status.device_cfg;
        return USB_ACK;
    case USB_STD_GET_DESCRIPTOR :
      if((req->wValue >> 8) == USB_DTYPE_STRING) {
        if((req->wValue & 0xFF) == 0x00) {
          udev.status.data_ptr = (uint8_t*)StringLangID;
          udev.status.data_count = StringLangID[0];
          return USB_ACK;
        } else if((req->wValue & 0xFF) == 0x01) {
          udev.status.data_ptr = (uint8_t*)StringVendor;
          udev.status.data_count = StringVendor[0];
          return USB_ACK;
        } else if((req->wValue & 0xFF) == 0x02) {
          udev.status.data_ptr = (uint8_t*)StringProduct;
          udev.status.data_count = StringProduct[0];
          return USB_ACK;
        } else if((req->wValue & 0xFF) == 0x03) {
          uint8_t *dsc = req->data;
          dsc[0] = 50;
          dsc[1] = USB_DTYPE_STRING;
          IntToUnicode(*(__IO uint32_t*)(0x1FFFF7AC), &dsc[2]);
          IntToUnicode(*(__IO uint32_t*)(0x1FFFF7B0), &dsc[18]);
          IntToUnicode(*(__IO uint32_t*)(0x1FFFF7B4), &dsc[34]);
          udev.status.data_count = 50;
          return USB_ACK;
        } else {
          return USB_FAIL;
        }
      } else if((req->wValue >> 8) == USB_DTYPE_DEVICE) {
        udev.status.data_ptr = (uint8_t*)USB_Descriptor;
        udev.status.data_count = USB_Descriptor[0];
        return USB_ACK;
      } else if((req->wValue >> 8) == USB_DTYPE_CONFIGURATION) {
        udev.status.data_ptr = (uint8_t*)USB_ConfigDescriptor;
        udev.status.data_count = sizeof(USB_ConfigDescriptor);
        return USB_ACK;
      }
      return USB_FAIL;
      
//      break;
    case USB_STD_GET_STATUS :
      req->data[0] = 0;
      req->data[1] = 0;
      return USB_ACK;
    case USB_STD_SET_ADDRESS :
      if(getinfo() & USBD_HW_ADDRFST) {
        USB_SetAddress(&udev, req);
      } else {
        udev.complete_callback = USB_SetAddress;
      }
      return USB_ACK;
    case USB_STD_SET_CONFIG :
      if(cdc_setconf(&udev, req->wValue) == USB_ACK) {
        udev.status.device_cfg = req->wValue;
        udev.status.device_state = (req->wValue) ? USB_STATE_CONFIGURED : USB_STATE_ADDRESSED;
        return USB_ACK;
      }
      return USB_FAIL;
    case USB_STD_SET_DESCRIPTOR :
        /* should be externally handled */
        break;
    case USB_STD_SET_FEATURE:
      break;
  }
  return USB_FAIL;
}

/** \brief Standard control request processing for interface
 * \param dev pointer to usb device
 * \param req pointer to control request
 * \return TRUE if request is handled
 */
USB_Response usbd_process_intrq(usbd_device *dev, usbd_ctlreq *req) {
    (void)dev;
    switch (req->bRequest) {
    case USB_STD_GET_STATUS:
        req->data[0] = 0;
        req->data[1] = 0;
        return USB_ACK;
    default:
        break;
    }
    return USB_FAIL;
}

/** \brief Standard control request processing for endpoint
 * \param dev pointer to usb device
 * \param req pointer to control request
 * \return TRUE if request is handled
 */
USB_Response usbd_process_eptrq(usbd_device *dev, usbd_ctlreq *req) {
    switch (req->bRequest) {
    case USB_STD_SET_FEATURE:
        USB_StallEP(req->wIndex, 1);
        return USB_ACK;
    case USB_STD_CLEAR_FEATURE:
        USB_StallEP(req->wIndex, 0);
        return USB_ACK;
    case USB_STD_GET_STATUS:
        req->data[0] = USB_StalledEP(req->wIndex) ? 1 : 0;
        req->data[1] = 0;
        return USB_ACK;
    default:
        break;
    }
    return USB_FAIL;
}

/** \brief Processing control request
 * \param dev pointer to usb device
 * \param req pointer to usb control request
 * \return TRUE if request is handled
 */
USB_Response usbd_process_request(usbd_device *dev, usbd_ctlreq *req) {
    /* processing control request by callback */
    USB_Response r = cdc_control(dev, req, &(dev->complete_callback));
    if (r != USB_FAIL) return r;
    /* continuing standard USB requests */
    switch (req->bmRequestType & (USB_REQ_TYPE | USB_REQ_RECIPIENT)) {
    case USB_REQ_STANDARD | USB_REQ_DEVICE:
        return usbd_process_devrq(req);
    case USB_REQ_STANDARD | USB_REQ_INTERFACE:
        return usbd_process_intrq(dev, req);
    case USB_REQ_STANDARD | USB_REQ_ENDPOINT:
        return usbd_process_eptrq(dev, req);
    default:
        break;
    }
    return USB_FAIL;
}


/** \brief Control endpoint stall (STALL PID)
 * \param dev pointer to usb device
 * \param ep endpoint number
 */
static void usbd_stall_pid(usbd_device *dev, uint8_t ep) {
    USB_StallEP(ep & 0x7F, 1);
    USB_StallEP(ep | 0x80, 1);
    dev->status.control_state = USB_CONTROL_IDLE;
}


/** \brief Control endpoint TX event processing
 * \param dev pointer to usb device
 * \param ep endpoint number
 */
static void usbd_process_eptx(usbd_device *dev, uint8_t ep) {
    int32_t _t;
    switch (dev->status.control_state) {
    case USB_CONTROL_TXPAYLOAD:
    case USB_CONTROL_TXDATA:
      _t = (dev->status.data_count < dev->status.ep0size) ? dev->status.data_count : dev->status.ep0size;
        ep_write(ep, dev->status.data_ptr, _t);
        dev->status.data_ptr += _t;
        dev->status.data_count -= _t;
        /* if all data is not sent */
        if (0 != dev->status.data_count) break;
        /* if last packet has a EP0 size and host awaiting for the more data ZLP should be sent*/
        /* if ZLP required, control state will be unchanged, therefore next TX event sends ZLP */
        if ( USB_CONTROL_TXDATA == dev->status.control_state || _t != dev->status.ep0size ) {
            dev->status.control_state = USB_CONTROL_LASTDATA; /* no ZLP required */
        }
        break;
    case USB_CONTROL_LASTDATA:
        dev->status.control_state = USB_CONTROL_STATUSOUT;
        break;
    case USB_CONTROL_STATUSIN:
        dev->status.control_state = USB_CONTROL_IDLE;
        return usbd_process_callback(dev);
    default:
        /* unexpected TX completion */
        /* just skipping it */
        break;
    }
}

/** \brief Control endpoint RX event processing
 * \param dev pointer to usb device
 * \param ep endpoint number
 */
static void usbd_process_eprx(usbd_device *dev, uint8_t ep) {
    uint16_t _t;
    usbd_ctlreq *const req = dev->status.data_buf;
    switch (dev->status.control_state) {
    case USB_CONTROL_IDLE:
        /* read SETUP packet, send STALL_PID if incorrect packet length */
        if (0x08 !=  ep_read(ep, req, dev->status.data_maxsize)) {
            return usbd_stall_pid(dev, ep);
        }
        dev->status.data_ptr = req->data;
        dev->status.data_count = req->wLength;
        /* processing request with no payload data*/
        if ((req->bmRequestType & USB_REQ_DEVTOHOST) || (0 == req->wLength)) break;
        /* checking available memory for DATA OUT stage */
        if (req->wLength > dev->status.data_maxsize) {
            return usbd_stall_pid(dev, ep);
        }
        /* continue DATA OUT stage */
        dev->status.control_state = USB_CONTROL_RXDATA;
        return;
    case USB_CONTROL_RXDATA:
        /*receive DATA OUT packet(s) */
        _t = ep_read(ep, dev->status.data_ptr, dev->status.data_count);
        if (dev->status.data_count < _t) {
        /* if received packet is large than expected */
        /* Must be error. Let's drop this request */
            return usbd_stall_pid(dev, ep);
        } else if (dev->status.data_count != _t) {
        /* if all data payload was not received yet */
            dev->status.data_count -= _t;
            dev->status.data_ptr += _t;
            return;
        }
        break;
    case USB_CONTROL_STATUSOUT:
        /* reading STATUS OUT data to buffer */
        ep_read(ep, dev->status.data_ptr, dev->status.data_maxsize);
        dev->status.control_state = USB_CONTROL_IDLE;
        return usbd_process_callback(dev);
    default:
        /* unexpected RX packet */
        return usbd_stall_pid(dev, ep);
    }
    /* usb request received. let's handle it */
    dev->status.data_ptr = req->data;
    dev->status.data_count = /*req->wLength;*/dev->status.data_maxsize;
    switch (usbd_process_request(dev, req)) {
    case USB_ACK:
        if (req->bmRequestType & USB_REQ_DEVTOHOST) {
            /* return data from function */
            if (dev->status.data_count >= req->wLength) {
                dev->status.data_count = req->wLength;
                dev->status.control_state = USB_CONTROL_TXDATA;
            } else {
                /* DATA IN packet smaller than requested */
                /* ZLP maybe wanted */
                dev->status.control_state = USB_CONTROL_TXPAYLOAD;
            }
            return usbd_process_eptx(dev, ep | 0x80);

        } else {
            /* confirming by ZLP in STATUS_IN stage */
            ep_write(ep | 0x80, 0, 0);
            dev->status.control_state = USB_CONTROL_STATUSIN;
        }
        break;
    case USB_NACK:
        dev->status.control_state = USB_CONTROL_STATUSIN;
        break;
    default:
        return usbd_stall_pid(dev, ep);
    }
}

/** \brief Control endpoint 0 event processing callback
 * \param dev usb device
 * \param event endpoint event
 */
void usbd_process_ep0(usbd_device *dev, uint8_t event, uint8_t ep) {
    switch (event) {
    case usbd_evt_epsetup:
        /* force switch to setup state */
        dev->status.control_state = USB_CONTROL_IDLE;
        dev->complete_callback = 0;
    case usbd_evt_eprx:
        return usbd_process_eprx(dev, ep);
    case usbd_evt_eptx:
        return usbd_process_eptx(dev, ep);
    default:
        break;
    }
}


/** \brief General event processing callback
 * \param dev usb device
 * \param evt usb event
 * \param ep active endpoint
 */
void usbd_process_evt(usbd_device *dev, uint8_t evt, uint8_t ep) {
  switch (evt) {
    case usbd_evt_eprx:
    case usbd_evt_eptx:
    case usbd_evt_epsetup:
      if(dev->endpoint[ep & 0x07]) dev->endpoint[ep & 0x07](dev, evt, ep);
      break;
    default:
      break;
  }
  if (dev->events[evt]) dev->events[evt](dev, evt, ep);
}
