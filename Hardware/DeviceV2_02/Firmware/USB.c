// Modified by Eduardo Viciana (zRed)

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
  0x01, 0x02,                             /* bcdDevice = 2.02 */
  1,                                      /* Index of string descriptor describing manufacturer */
  2,                                      /* Index of string descriptor describing product */
  3,                                      /* Index of string descriptor describing the device's serial number */
  0x01                                    /* bNumConfigurations */
};
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


volatile USB_Device_t udev;
uint32_t	ubuf[0x20] __attribute__ ((aligned (4)));
//uint8_t     fifo[0x200];
uint32_t    fpos = 0;


#define USB_EP_SWBUF_TX     USB_EP_DTOG_RX
#define USB_EP_SWBUF_RX     USB_EP_DTOG_TX

#define EP_TOGGLE_SET(epr, bits, mask) *(epr) = (*(epr) ^ (bits)) & (USB_EPREG_MASK | (mask))

#define EP_TX_STALL(epr)    EP_TOGGLE_SET((epr), USB_EP_TX_STALL,                   USB_EPTX_STAT)
#define EP_RX_STALL(epr)    EP_TOGGLE_SET((epr), USB_EP_RX_STALL,                   USB_EPRX_STAT)
#define EP_TX_UNSTALL(epr)  EP_TOGGLE_SET((epr), USB_EP_TX_NAK,                     USB_EPTX_STAT | USB_EP_DTOG_TX)
#define EP_RX_UNSTALL(epr)  EP_TOGGLE_SET((epr), USB_EP_RX_VALID,                   USB_EPRX_STAT | USB_EP_DTOG_RX)
#define EP_DTX_UNSTALL(epr) EP_TOGGLE_SET((epr), USB_EP_TX_VALID,                   USB_EPTX_STAT | USB_EP_DTOG_TX | USB_EP_SWBUF_TX)
#define EP_DRX_UNSTALL(epr) EP_TOGGLE_SET((epr), USB_EP_RX_VALID | USB_EP_SWBUF_RX, USB_EPRX_STAT | USB_EP_DTOG_RX | USB_EP_SWBUF_RX)
#define EP_TX_VALID(epr)    EP_TOGGLE_SET((epr), USB_EP_TX_VALID,                   USB_EPTX_STAT)
#define EP_RX_VALID(epr)    EP_TOGGLE_SET((epr), USB_EP_RX_VALID,                   USB_EPRX_STAT)

#define STATUS_VAL(x)   (USBD_HW_BC | (x))

#ifndef USB_PMASIZE
#warning PMA memory size is not defined. Use 1k by default
#define USB_PMASIZE 0x400
#endif



typedef struct {
  uint16_t addr;
  uint16_t cnt;
} pma_rec;

typedef union pma_table {
  struct {
    pma_rec tx;
    pma_rec rx;
  };
  struct {
    pma_rec tx0;
    pma_rec tx1;
  };
  struct {
    pma_rec rx0;
    pma_rec rx1;
  };
} pma_table;

/** \brief Helper function. Returns pointer to the buffer descriptor table.
 */
inline static pma_table *EPT(uint8_t ep) {
  return (pma_table*) ((ep & 0x07) * 8 + USB_PMAADDR);
}

/** \brief Helper function. Returns pointer to the endpoint control register.
 */
inline static volatile uint16_t *EPR(uint8_t ep) {
  return (uint16_t*) ((ep & 0x07) * 4 + USB_BASE);
}

/** \brief Helper function. Returns next available PMA buffer.
 *
 * \param sz uint16_t Requested buffer size.
 * \return uint16_t Buffer address for PMA table.
 * \note PMA buffers grown from top to bottom like stack.
 */
static uint16_t get_next_pma(uint16_t sz) {
  unsigned _result = USB_PMASIZE;
  for (int i = 0; i < 8; i++) {
    pma_table *tbl = EPT(i);
    if ((tbl->rx.addr) && (tbl->rx.addr < _result)) _result = tbl->rx.addr;
    if ((tbl->tx.addr) && (tbl->tx.addr < _result)) _result = tbl->tx.addr;
  }
  return (_result < (0x020 + sz)) ? 0 : (_result - sz);
}

void USB_StallEP(uint8_t ep, bool stall) {
  volatile uint16_t *reg = EPR(ep);
  /* ISOCHRONOUS endpoint can't be stalled or unstalled */
  if(USB_EP_ISOCHRONOUS == (*reg & USB_EP_T_FIELD)) return;
  /* If it's an IN endpoint */
  if(ep & 0x80) {
    /* DISABLED endpoint can't be stalled or unstalled */
    if(USB_EP_TX_DIS == (*reg & USB_EPTX_STAT)) return;
    if(stall) {
      EP_TX_STALL(reg);
    } else {
      /* if it's a doublebuffered endpoint */
      if((USB_EP_KIND | USB_EP_BULK) == (*reg & (USB_EP_T_FIELD | USB_EP_KIND))) {
        /* set endpoint to VALID and clear DTOG_TX & SWBUF_TX */
        EP_DTX_UNSTALL(reg);
      } else {
        /* set endpoint to NAKED and clear DTOG_TX */
        EP_TX_UNSTALL(reg);
      }
    }
  } else {
    if(USB_EP_RX_DIS == (*reg & USB_EPRX_STAT)) return;
    if(stall) {
      EP_RX_STALL(reg);
    } else {
      /* if it's a doublebuffered endpoint */
      if((USB_EP_KIND | USB_EP_BULK) == (*reg & (USB_EP_T_FIELD | USB_EP_KIND))) {
        /* set endpoint to VALID, clear DTOG_RX, set SWBUF_RX */
        EP_DRX_UNSTALL(reg);
      } else {
        /* set endpoint to VALID and clear DTOG_RX */
        EP_RX_UNSTALL(reg);
      }
    }
  }
}
bool USB_StalledEP(uint8_t ep) {
  if(ep & 0x80) {
    return (USB_EP_TX_STALL == (USB_EPTX_STAT & *EPR(ep)));
  } else {
    return (USB_EP_RX_STALL == (USB_EPRX_STAT & *EPR(ep)));
  }
}
void USB_Enable(bool enable) {
  if (enable) {
    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
    RCC->APB1RSTR |= RCC_APB1RSTR_USBRST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USBRST;
    USB->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM | USB_CNTR_ERRM //|
//#if !defined(USBD_SOF_DISABLED)
//            USB_CNTR_SOFM |
//#endif
//            USB_CNTR_SUSPM | USB_CNTR_WKUPM
            ;
  } else if (RCC->APB1ENR & RCC_APB1ENR_USBEN) {
    USB->BCDR = 0;
    RCC->APB1RSTR |= RCC_APB1RSTR_USBRST;
    RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
  }
}
uint8_t USB_Connect(bool connect) {
  uint8_t res;
  USB->BCDR = USB_BCDR_BCDEN | USB_BCDR_DCDEN;
  if(USB->BCDR & USB_BCDR_DCDET) {
    USB->BCDR = USB_BCDR_BCDEN | USB_BCDR_PDEN;
    if(USB->BCDR & USB_BCDR_PS2DET) {
      res = usbd_lane_unk;
    } else if (USB->BCDR & USB_BCDR_PDET) {
      USB->BCDR = USB_BCDR_BCDEN | USB_BCDR_SDEN;
      if(USB->BCDR & USB_BCDR_SDET) {
        res = usbd_lane_dcp;
      } else {
        res = usbd_lane_cdp;
      }
    } else {
      res = usbd_lane_sdp;
    }
  } else {
    res = usbd_lane_dsc;
  }
  USB->BCDR = (connect) ? USB_BCDR_DPPU : 0;
  return res;
}
bool USB_ConfigEP(uint8_t ep, uint8_t eptype, uint16_t epsize) {
  volatile uint16_t *reg = EPR(ep);
  pma_table *tbl = EPT(ep);
  /* epsize should be 16-bit aligned */
  if (epsize & 0x01) epsize++;

  switch(eptype) {
    case USB_EPTYPE_CONTROL:
      *reg = USB_EP_CONTROL | (ep & 0x07);
      break;
    case USB_EPTYPE_ISOCHRONUS:
      *reg = USB_EP_ISOCHRONOUS | (ep & 0x07);
      break;
    case USB_EPTYPE_BULK:
      *reg = USB_EP_BULK | (ep & 0x07);
      break;
    case USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF:
      *reg = USB_EP_BULK | USB_EP_KIND | (ep & 0x07);
      break;
    default:
      *reg = USB_EP_INTERRUPT | (ep & 0x07);
      break;
  }
  /* if it TX or CONTROL endpoint */
  if((ep & 0x80) || (eptype == USB_EPTYPE_CONTROL)) {
    uint16_t _pma;
    _pma = get_next_pma(epsize);
    if(_pma == 0) return false;
    tbl->tx.addr = _pma;
    tbl->tx.cnt = 0;
    if((eptype == USB_EPTYPE_ISOCHRONUS) || (eptype == (USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF))) {
      _pma = get_next_pma(epsize);
      if (_pma == 0) return false;
      tbl->tx1.addr = _pma;
      tbl->tx1.cnt = 0;
      EP_DTX_UNSTALL(reg);
    } else {
      EP_TX_UNSTALL(reg);
    }
  }
  if(!(ep & 0x80)) {
    uint16_t _rxcnt;
    uint16_t _pma;
    if(epsize > 62) {
      if (epsize & 0x1F) {
        epsize &= 0x1F;
      } else {
        epsize -= 0x20;
      }
      _rxcnt = 0x8000 | (epsize << 5);
      epsize += 0x20;
    } else {
      _rxcnt = epsize << 9;
    }
    _pma = get_next_pma(epsize);
    if (_pma == 0) return false;
    tbl->rx.addr = _pma;
    tbl->rx.cnt = _rxcnt;
    if((eptype == USB_EPTYPE_ISOCHRONUS) || (eptype == (USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF))) {
      _pma = get_next_pma(epsize);
      if (_pma == 0) return false;
      tbl->rx0.addr = _pma;
      tbl->rx0.cnt = _rxcnt;
      EP_DRX_UNSTALL(reg);
    } else {
      EP_RX_UNSTALL(reg);
    }
  }
  return true;
}
void USB_DeconfigEP(uint8_t ep) {
  pma_table *ept = EPT(ep);
  *EPR(ep) &= ~USB_EPREG_MASK;
  ept->rx.addr = 0;
  ept->rx.cnt = 0;
  ept->tx.addr = 0;
  ept->tx.cnt = 0;
}
void USB_EP_Register(uint8_t ep, USB_Event_Callback_t callback) {
  udev.Endpoint[ep & 0x07] = callback;
}
uint16_t USB_PMA_Read(uint8_t *buf, uint16_t blen, pma_rec *rx) {
  uint16_t *pma = (void*) (USB_PMAADDR + rx->addr);
  uint16_t rxcnt = rx->cnt & 0x03FF;
  rx->cnt &= ~0x3FF;
  if(blen > rxcnt) blen = rxcnt;
  rxcnt = blen;
  while (blen) {
    uint16_t _t = *pma;
    *buf++ = _t & 0xFF;
    if (--blen) {
      *buf++ = _t >> 8;
      pma++;
      blen--;
    } else break;
  }
  return rxcnt;
}
int32_t USB_EP_Read(uint8_t ep, void *buf, uint16_t blen) {
  pma_table *tbl = EPT(ep);
  volatile uint16_t *reg = EPR(ep);
  switch (*reg & (USB_EPRX_STAT | USB_EP_T_FIELD | USB_EP_KIND)) {
      /* doublebuffered bulk endpoint */
    case (USB_EP_RX_VALID | USB_EP_BULK | USB_EP_KIND):
      /* switching SWBUF if EP is NAKED */
      switch(*reg & (USB_EP_DTOG_RX | USB_EP_SWBUF_RX)) {
        case 0:
        case (USB_EP_DTOG_RX | USB_EP_SWBUF_RX):
          *reg = (*reg & USB_EPREG_MASK) | USB_EP_SWBUF_RX;
          break;
        default:
          break;
      }
      if(*reg & USB_EP_SWBUF_RX) {
        return USB_PMA_Read(buf, blen, &(tbl->rx1));
      } else {
        return USB_PMA_Read(buf, blen, &(tbl->rx0));
      }
      /* isochronous endpoint */
    case (USB_EP_RX_VALID | USB_EP_ISOCHRONOUS):
      if (*reg & USB_EP_DTOG_RX) {
        return USB_PMA_Read(buf, blen, &(tbl->rx1));
      } else {
        return USB_PMA_Read(buf, blen, &(tbl->rx0));
      }
      /* regular endpoint */
    case (USB_EP_RX_NAK | USB_EP_BULK):
    case (USB_EP_RX_NAK | USB_EP_CONTROL):
    case (USB_EP_RX_NAK | USB_EP_INTERRUPT):
    {
      int32_t res = USB_PMA_Read(buf, blen, &(tbl->rx));
      /* setting endpoint to VALID state */
      EP_RX_VALID(reg);
      return res;
    }
      /* invalid or not ready */
    default:
      return -1;
  }
}
static void USB_PMA_Write(uint8_t *buf, uint16_t blen, pma_rec *tx) {
  uint16_t *pma = (void*) (USB_PMAADDR + tx->addr);
  tx->cnt = blen;
  while (blen > 1) {
    *pma++ = buf[1] << 8 | buf[0];
    buf += 2;
    blen -= 2;
  }
  if (blen) *pma = *buf;
}
int32_t USB_EP_Write(uint8_t ep, void *buf, uint16_t blen) {
  pma_table *tbl = EPT(ep);
  volatile uint16_t *reg = EPR(ep);
  switch(*reg & (USB_EPTX_STAT | USB_EP_T_FIELD | USB_EP_KIND)) {
      /* doublebuffered bulk endpoint */
    case (USB_EP_TX_NAK | USB_EP_BULK | USB_EP_KIND):
      if(*reg & USB_EP_SWBUF_TX) {
        USB_PMA_Write(buf, blen, &(tbl->tx1));
      } else {
        USB_PMA_Write(buf, blen, &(tbl->tx0));
      }
      *reg = (*reg & USB_EPREG_MASK) | USB_EP_SWBUF_TX;
      break;
      /* isochronous endpoint */
    case(USB_EP_TX_VALID | USB_EP_ISOCHRONOUS):
      if (!(*reg & USB_EP_DTOG_TX)) {
        USB_PMA_Write(buf, blen, &(tbl->tx1));
      } else {
        USB_PMA_Write(buf, blen, &(tbl->tx0));
      }
      break;
      /* regular endpoint */
    case(USB_EP_TX_NAK | USB_EP_BULK):
    case(USB_EP_TX_NAK | USB_EP_CONTROL):
    case(USB_EP_TX_NAK | USB_EP_INTERRUPT):
      USB_PMA_Write(buf, blen, &(tbl->tx));
      EP_TX_VALID(reg);
      break;
      /* invalid or not ready */
    default:
      return -1;
  }
  return blen;
}
void USB_ISR() {
  uint8_t _ev, _ep;
  uint16_t _istr = USB->ISTR;
  _ep = _istr & USB_ISTR_EP_ID;
  if(_istr & USB_ISTR_CTR) {
    volatile uint16_t *reg = EPR(_ep);
    if(*reg & USB_EP_CTR_TX) {
      *reg &= (USB_EPREG_MASK ^ USB_EP_CTR_TX);
      _ep |= 0x80;
      _ev = usbd_evt_eptx;
    } else {
      *reg &= (USB_EPREG_MASK ^ USB_EP_CTR_RX);
      _ev = (*reg & USB_EP_SETUP) ? usbd_evt_epsetup : usbd_evt_eprx;
    }
  } else if(_istr & USB_ISTR_RESET) {
    USB->ISTR &= ~USB_ISTR_RESET;
    USB->BTABLE = 0;
    for(int i = 0; i < 8; i++) USB_DeconfigEP(i);
    udev.Status.device_state = USB_STATE_DEFAULT;
    udev.Status.control_state = USB_CONTROL_IDLE;
    udev.Status.device_cfg = 0;
    USB_ConfigEP(0, USB_EPTYPE_CONTROL, udev.Status.ep0size);
    udev.Endpoint[0] = usbd_process_ep0;
    USB->DADDR = USB_DADDR_EF;
    return;
//#if !defined(USBD_SOF_DISABLED)
//  } else if(_istr & USB_ISTR_SOF) {
//    _ev = usbd_evt_sof;
//    USB->ISTR &= ~USB_ISTR_SOF;
//#endif
//  } else if(_istr & USB_ISTR_WKUP) {
//    _ev = usbd_evt_wkup;
//    USB->CNTR &= ~USB_CNTR_FSUSP;
//    USB->ISTR &= ~USB_ISTR_WKUP;
//  } else if (_istr & USB_ISTR_SUSP) {
//    _ev = usbd_evt_susp;
//    USB->CNTR |= USB_CNTR_FSUSP;
//    USB->ISTR &= ~USB_ISTR_SUSP;
  } else if (_istr & USB_ISTR_ERR) {
    USB->ISTR &= ~USB_ISTR_ERR;
    _ev = usbd_evt_error;
  } else {
    return;
  }

  switch(_ev) {
    case usbd_evt_eprx:
    case usbd_evt_eptx:
    case usbd_evt_epsetup:
      if(udev.Endpoint[_ep & 0x07]) udev.Endpoint[_ep & 0x07](_ev, _ep);
      break;
    default:
      break;
  }
//  if(udev.Events[_ev]) udev.Events[_ev](_ev, _ep);
  
  
//  usbd_process_evt(&udev, _ev, _ep);
}

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




USB_Response cdc_control(usbd_ctlreq *req, USB_Request_Callback_t *callback) {
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
USB_Response cdc_setconf ( uint8_t cfg) {
  switch (cfg) {
    case 0:
        /* deconfiguring device */
//        USB_DeconfigEP(CDC_NTF_EP);
        USB_DeconfigEP(CDC_TXD_EP);
//        USB_DeconfigEP(CDC_RXD_EP);
//        usbd_reg_endpoint(dev, CDC_RXD_EP, 0);
        USB_EP_Register(CDC_TXD_EP, 0);
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
        USB_EP_Register(CDC_TXD_EP, cdc_txonly);
//#endif
        USB_EP_Write(CDC_TXD_EP, 0, 0);
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
    udev.Status.ep0size = CDC_EP0_SIZE;
    udev.Status.data_ptr = ubuf;
    udev.Status.data_buf = ubuf;
    udev.Status.data_maxsize = sizeof(ubuf) - __builtin_offsetof(usbd_ctlreq, data);
}
  void usbd_process_ep0(uint8_t event, uint8_t ep);

/** \brief Callback that sets USB device address
 * \param dev pointer to usb device
 * \param req pointer to usb control request data
 * \return none
 */
void USB_SetAddress(usbd_ctlreq *req) {
  USB->DADDR = USB_DADDR_EF | req->wValue;
  udev.Status.device_state = (req->wValue) ? USB_STATE_ADDRESSED : USB_STATE_DEFAULT;
}

/** \brief Control transfer completion callback processing
 * \param dev pointer to the usb device
 * \return none
 */
static void usbd_process_callback() {
  if(udev.Complete_Callback) {
    udev.Complete_Callback(udev.Status.data_buf);
    udev.Complete_Callback = 0;
  }
}

/** \brief SET_CONFIG request processing
 * \param dev usbd_device
 * \param config config number from request
 * \return usbd_ack if success
 */
USB_Response cdc_setconf(uint8_t cfg);
//USB_Response usbd_configure(usbd_device *dev, uint8_t config) {
//        if (cdc_setconf(dev, config) == USB_ACK) {
//            dev->status.device_cfg = config;
//            dev->status.device_state = (config) ? USB_STATE_CONFIGURED : USB_STATE_ADDRESSED;
//            return USB_ACK;
//        }
//    return USB_FAIL;
//}

uint32_t getinfo() {
  if(!(RCC->APB1ENR & RCC_APB1ENR_USBEN)) return STATUS_VAL(0);
  if(USB->BCDR & USB_BCDR_DPPU) return STATUS_VAL(USBD_HW_ENABLED | USBD_HW_SPEED_FS);
  return STATUS_VAL(USBD_HW_ENABLED);
}
/** \brief Standard control request processing for device
 * \param dev pointer to usb device
 * \param req pointer to control request
 * \return TRUE if request is handled
 */
//USB_Response cdc_getdesc (usbd_ctlreq *req, void **address, uint16_t *length);

USB_Response usbd_process_devrq(usbd_ctlreq *req) {
  switch (req->bRequest) {
    case USB_STD_CLEAR_FEATURE:
      /* not yet supported */
      break;
    case USB_STD_GET_CONFIG:
      req->data[0] = udev.Status.device_cfg;
      return USB_ACK;
    case USB_STD_GET_DESCRIPTOR:
      if ((req->wValue >> 8) == USB_DTYPE_STRING) {
        if ((req->wValue & 0xFF) == 0x00) {
          udev.Status.data_ptr = (uint8_t*) StringLangID;
          udev.Status.data_count = StringLangID[0];
          return USB_ACK;
        } else if ((req->wValue & 0xFF) == 0x01) {
          udev.Status.data_ptr = (uint8_t*) StringVendor;
          udev.Status.data_count = StringVendor[0];
          return USB_ACK;
        } else if ((req->wValue & 0xFF) == 0x02) {
          udev.Status.data_ptr = (uint8_t*) StringProduct;
          udev.Status.data_count = StringProduct[0];
          return USB_ACK;
        } else if ((req->wValue & 0xFF) == 0x03) {
          uint8_t *dsc = req->data;
          dsc[0] = 50;
          dsc[1] = USB_DTYPE_STRING;
          IntToUnicode(*(__IO uint32_t*) (0x1FFFF7AC), &dsc[2]);
          IntToUnicode(*(__IO uint32_t*) (0x1FFFF7B0), &dsc[18]);
          IntToUnicode(*(__IO uint32_t*) (0x1FFFF7B4), &dsc[34]);
          udev.Status.data_count = 50;
          return USB_ACK;
        } else {
          return USB_FAIL;
        }
      } else if ((req->wValue >> 8) == USB_DTYPE_DEVICE) {
        udev.Status.data_ptr = (uint8_t*) USB_Descriptor;
        udev.Status.data_count = USB_Descriptor[0];
        return USB_ACK;
      } else if ((req->wValue >> 8) == USB_DTYPE_CONFIGURATION) {
        udev.Status.data_ptr = (uint8_t*) USB_ConfigDescriptor;
        udev.Status.data_count = sizeof(USB_ConfigDescriptor);
        return USB_ACK;
      }
      return USB_FAIL;

      //      break;
    case USB_STD_GET_STATUS:
      req->data[0] = 0;
      req->data[1] = 0;
      return USB_ACK;
    case USB_STD_SET_ADDRESS:
      if (getinfo() & USBD_HW_ADDRFST) {
        USB_SetAddress(req);
      } else {
        udev.Complete_Callback = USB_SetAddress;
      }
      return USB_ACK;
    case USB_STD_SET_CONFIG:
      if (cdc_setconf(req->wValue) == USB_ACK) {
        udev.Status.device_cfg = req->wValue;
        udev.Status.device_state = (req->wValue) ? USB_STATE_CONFIGURED : USB_STATE_ADDRESSED;
        return USB_ACK;
      }
      return USB_FAIL;
    case USB_STD_SET_DESCRIPTOR:
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
USB_Response usbd_process_intrq(usbd_ctlreq *req) {
//  (void) dev;
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
USB_Response usbd_process_eptrq(usbd_ctlreq *req) {
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
USB_Response usbd_process_request(usbd_ctlreq *req) {
  /* processing control request by callback */
  USB_Response r = cdc_control(req, &(udev.Complete_Callback));
  if (r != USB_FAIL) return r;
  /* continuing standard USB requests */
  switch (req->bmRequestType & (USB_REQ_TYPE | USB_REQ_RECIPIENT)) {
    case USB_REQ_STANDARD | USB_REQ_DEVICE:
      return usbd_process_devrq(req);
    case USB_REQ_STANDARD | USB_REQ_INTERFACE:
      return usbd_process_intrq(req);
    case USB_REQ_STANDARD | USB_REQ_ENDPOINT:
      return usbd_process_eptrq(req);
    default:
      break;
  }
  return USB_FAIL;
}

/** \brief Control endpoint stall (STALL PID)
 * \param ep endpoint number
 */
static void usbd_stall_pid(uint8_t ep) {
  USB_StallEP(ep & 0x7F, 1);
  USB_StallEP(ep | 0x80, 1);
  udev.Status.control_state = USB_CONTROL_IDLE;
}

/** \brief Control endpoint TX event processing
 * \param ep endpoint number
 */
static void usbd_process_eptx(uint8_t ep) {
  int32_t _t;
  switch (udev.Status.control_state) {
    case USB_CONTROL_TXPAYLOAD:
    case USB_CONTROL_TXDATA:
      _t = (udev.Status.data_count < udev.Status.ep0size) ? udev.Status.data_count : udev.Status.ep0size;
      USB_EP_Write(ep, udev.Status.data_ptr, _t);
      udev.Status.data_ptr += _t;
      udev.Status.data_count -= _t;
      /* if all data is not sent */
      if (0 != udev.Status.data_count) break;
      /* if last packet has a EP0 size and host awaiting for the more data ZLP should be sent*/
      /* if ZLP required, control state will be unchanged, therefore next TX event sends ZLP */
      if (USB_CONTROL_TXDATA == udev.Status.control_state || _t != udev.Status.ep0size) {
        udev.Status.control_state = USB_CONTROL_LASTDATA; /* no ZLP required */
      }
      break;
    case USB_CONTROL_LASTDATA:
      udev.Status.control_state = USB_CONTROL_STATUSOUT;
      break;
    case USB_CONTROL_STATUSIN:
      udev.Status.control_state = USB_CONTROL_IDLE;
      return usbd_process_callback();
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
static void usbd_process_eprx(uint8_t ep) {
  uint16_t _t;
  usbd_ctlreq * const req = udev.Status.data_buf;
  switch (udev.Status.control_state) {
    case USB_CONTROL_IDLE:
      /* read SETUP packet, send STALL_PID if incorrect packet length */
      if (0x08 != USB_EP_Read(ep, req, udev.Status.data_maxsize)) {
        return usbd_stall_pid(ep);
      }
      udev.Status.data_ptr = req->data;
      udev.Status.data_count = req->wLength;
      /* processing request with no payload data*/
      if ((req->bmRequestType & USB_REQ_DEVTOHOST) || (0 == req->wLength)) break;
      /* checking available memory for DATA OUT stage */
      if (req->wLength > udev.Status.data_maxsize) {
        return usbd_stall_pid(ep);
      }
      /* continue DATA OUT stage */
      udev.Status.control_state = USB_CONTROL_RXDATA;
      return;
    case USB_CONTROL_RXDATA:
      /*receive DATA OUT packet(s) */
      _t = USB_EP_Read(ep, udev.Status.data_ptr, udev.Status.data_count);
      if (udev.Status.data_count < _t) {
        /* if received packet is large than expected */
        /* Must be error. Let's drop this request */
        return usbd_stall_pid(ep);
      } else if (udev.Status.data_count != _t) {
        /* if all data payload was not received yet */
        udev.Status.data_count -= _t;
        udev.Status.data_ptr += _t;
        return;
      }
      break;
    case USB_CONTROL_STATUSOUT:
      /* reading STATUS OUT data to buffer */
      USB_EP_Read(ep, udev.Status.data_ptr, udev.Status.data_maxsize);
      udev.Status.control_state = USB_CONTROL_IDLE;
      return usbd_process_callback();
    default:
      /* unexpected RX packet */
      return usbd_stall_pid(ep);
  }
  /* usb request received. let's handle it */
  udev.Status.data_ptr = req->data;
  udev.Status.data_count = /*req->wLength;*/udev.Status.data_maxsize;
  switch (usbd_process_request(req)) {
    case USB_ACK:
      if (req->bmRequestType & USB_REQ_DEVTOHOST) {
        /* return data from function */
        if (udev.Status.data_count >= req->wLength) {
          udev.Status.data_count = req->wLength;
          udev.Status.control_state = USB_CONTROL_TXDATA;
        } else {
          /* DATA IN packet smaller than requested */
          /* ZLP maybe wanted */
          udev.Status.control_state = USB_CONTROL_TXPAYLOAD;
        }
        return usbd_process_eptx(ep | 0x80);

      } else {
        /* confirming by ZLP in STATUS_IN stage */
        USB_EP_Write(ep | 0x80, 0, 0);
        udev.Status.control_state = USB_CONTROL_STATUSIN;
      }
      break;
    case USB_NACK:
      udev.Status.control_state = USB_CONTROL_STATUSIN;
      break;
    default:
      return usbd_stall_pid(ep);
  }
}

/** \brief Control endpoint 0 event processing callback
 * \param dev usb device
 * \param event endpoint event
 */
void usbd_process_ep0(uint8_t event, uint8_t ep) {
  switch (event) {
    case usbd_evt_epsetup:
      /* force switch to setup state */
      udev.Status.control_state = USB_CONTROL_IDLE;
      udev.Complete_Callback = 0;
    case usbd_evt_eprx:
      return usbd_process_eprx(ep);
    case usbd_evt_eptx:
      return usbd_process_eptx(ep);
    default:
      break;
  }
}