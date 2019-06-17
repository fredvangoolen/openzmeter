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



static void usbd_process_ep0 (usbd_device *dev, uint8_t event, uint8_t ep);



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
USB_Response usbd_configure(usbd_device *dev, uint8_t config) {
        if (cdc_setconf(dev, config) == USB_ACK) {
            dev->status.device_cfg = config;
            dev->status.device_state = (config) ? USB_STATE_CONFIGURED : USB_STATE_ADDRESSED;
            return USB_ACK;
        }
    return USB_FAIL;
}


/** \brief Standard control request processing for device
 * \param dev pointer to usb device
 * \param req pointer to control request
 * \return TRUE if request is handled
 */
USB_Response cdc_getdesc (usbd_ctlreq *req, void **address, uint16_t *length);
USB_Response usbd_process_devrq (usbd_device *dev, usbd_ctlreq *req) {
    switch (req->bRequest) {
    case USB_STD_CLEAR_FEATURE:
        /* not yet supported */
        break;
    case USB_STD_GET_CONFIG:
        req->data[0] = dev->status.device_cfg;
        return USB_ACK;
    case USB_STD_GET_DESCRIPTOR:
        if (req->wValue == ((USB_DTYPE_STRING << 8) | INTSERIALNO_DESCRIPTOR )) {
            dev->status.data_count = get_serialno_desc(req->data);
            return USB_ACK;
        } else {
//            if (dev->descriptor_callback) {
                return cdc_getdesc(req, &(dev->status.data_ptr), &(dev->status.data_count));
//            }
        }
        break;
    case USB_STD_GET_STATUS:
        req->data[0] = 0;
        req->data[1] = 0;
        return USB_ACK;
    case USB_STD_SET_ADDRESS:
        if (getinfo(dev) & USBD_HW_ADDRFST) {
            USB_SetAddress(dev, req);
        } else {
            dev->complete_callback = USB_SetAddress;
        }
        return USB_ACK;
    case USB_STD_SET_CONFIG:
        return usbd_configure(dev, req->wValue);
    case USB_STD_SET_DESCRIPTOR:
        /* should be externally handled */
        break;
    case USB_STD_SET_FEATURE:
        /* not yet supported */
        break;
    default:
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
        return usbd_process_devrq(dev, req);
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
static void usbd_process_ep0 (usbd_device *dev, uint8_t event, uint8_t ep) {
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
static void usbd_process_evt(usbd_device *dev, uint8_t evt, uint8_t ep) {
    switch (evt) {
    case USB_EVENT_RESET:
    dev->status.device_state = USB_STATE_DEFAULT;
    dev->status.control_state = USB_CONTROL_IDLE;
    dev->status.device_cfg = 0;
    USB_ConfigEP(0, USB_EPTYPE_CONTROL, dev->status.ep0size);
    dev->endpoint[0] = usbd_process_ep0;
    USB->DADDR = USB_DADDR_EF;
        break;
    case usbd_evt_eprx:
    case usbd_evt_eptx:
    case usbd_evt_epsetup:
        if (dev->endpoint[ep & 0x07]) dev->endpoint[ep & 0x07](dev, evt, ep);
        break;
    default:
        break;
    }
    if (dev->events[evt]) dev->events[evt](dev, evt, ep);
}

 __attribute__((externally_visible)) void usbd_poll(usbd_device *dev) {
    evt_poll(dev, usbd_process_evt);
}