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
// Configuration -------------------------------------------------------------------------------------------------------
#define CDC_EP0_SIZE   0x08
#define USB_PMASIZE   0x400

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
  0x20, 0x00,                             /* wTotalLength:no of returned bytes */
  0x01,                                   /* bNumInterfaces: 1 interface */
  0x01,                                   /* bConfigurationValue: Configuration value */
  0x00,                                   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,                                   /* bmAttributes: self powered */
  0x00,                                   /* MaxPower 0 mA */

  0x09,                                   /* bLength: Interface Descriptor size */
  USB_DTYPE_INTERFACE,                    /* bDescriptorType: Interface */
  0x00,                                   /* bInterfaceNumber: Number of Interface */
  0x00,                                   /* bAlternateSetting: Alternate setting */
  0x02,                                   /* bNumEndpoints: One endpoints used */
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

  0x07,                                   /* bLength: Endpoint Descriptor size */
  USB_DTYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  0x02,                                   /* bEndpointAddress: (OUT2) */
  0x02,                                   /* bmAttributes: Bulk */
  0x02, 0x00,                             /* wMaxPacketSize: */
  0x00                                    /* bInterval: ignore for Bulk transfer */
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
// Variables -----------------------------------------------------------------------------------------------------------
USB_Device_t USB_Device;
uint8_t      USB_Buffer[32] __attribute__ ((aligned (4)));

// Functions -----------------------------------------------------------------------------------------------------------
PMA_Table_t *EPT(uint8_t ep) {
  return (PMA_Table_t*)((ep & 0x07) * 8 + USB_PMAADDR);
}
volatile uint16_t *EPR(uint8_t ep) {
  return (uint16_t*) ((ep & 0x07) * 4 + USB_BASE);
}
void         USB_IntToUnicode(uint32_t value, uint8_t *pbuf) {
  for(uint8_t i = 0; i < 8; i++) {
    *(pbuf++) = (value & 0x0F) + (((value & 0x0F) < 0x0A) ? '0' : 'A' - 10);
    *(pbuf++) = 0;
    value = value >> 4;
  }
}
uint16_t     USB_GetNextPMA(uint16_t sz) {
  unsigned _result = USB_PMASIZE;
  for(int i = 0; i < 8; i++) {
    PMA_Table_t *tbl = EPT(i);
    if ((tbl->rx.addr) && (tbl->rx.addr < _result)) _result = tbl->rx.addr;
    if ((tbl->tx.addr) && (tbl->tx.addr < _result)) _result = tbl->tx.addr;
  }
  return (_result < (0x020 + sz)) ? 0 : (_result - sz);
}
void         USB_StallEP(uint8_t ep, bool stall) {
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
bool         USB_StalledEP(uint8_t ep) {
  if(ep & 0x80) {
    return (USB_EP_TX_STALL == (USB_EPTX_STAT & *EPR(ep)));
  } else {
    return (USB_EP_RX_STALL == (USB_EPRX_STAT & *EPR(ep)));
  }
}
void         USB_Init() {
  USB_Device.Status.ep0size = CDC_EP0_SIZE;
  USB_Device.Status.data_ptr = USB_Buffer;
  USB_Device.Status.data_buf = USB_Buffer;
  USB_Device.Status.data_maxsize = sizeof(USB_Buffer) - __builtin_offsetof(USB_ControlRequest_t, data);
}
void         USB_Enable(bool enable) {
  if (enable) {
    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
    RCC->APB1RSTR |= RCC_APB1RSTR_USBRST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USBRST;
    USB->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM | USB_CNTR_ERRM |
//#if !defined(USBD_SOF_DISABLED)
//            USB_CNTR_SOFM |
//#endif
            USB_CNTR_SUSPM | USB_CNTR_WKUPM
            ;
  } else if (RCC->APB1ENR & RCC_APB1ENR_USBEN) {
    USB->BCDR = 0;
    RCC->APB1RSTR |= RCC_APB1RSTR_USBRST;
    RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
  }
}
uint32_t     USB_GetInfo() {
  if(!(RCC->APB1ENR & RCC_APB1ENR_USBEN)) return STATUS_VAL(0);
  if(USB->BCDR & USB_BCDR_DPPU) return STATUS_VAL(USBD_HW_ENABLED | USBD_HW_SPEED_FS);
  return STATUS_VAL(USBD_HW_ENABLED);
}
uint8_t      USB_Connect(bool connect) {
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
bool         USB_ConfigEP(uint8_t ep, uint8_t eptype, uint16_t epsize) {
  volatile uint16_t *reg = EPR(ep);
  PMA_Table_t *tbl = EPT(ep);
  /* epsize should be 16-bit aligned */
  if(epsize & 0x01) epsize++;
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
    _pma = USB_GetNextPMA(epsize);
    if(_pma == 0) return false;
    tbl->tx.addr = _pma;
    tbl->tx.cnt = 0;
    if((eptype == USB_EPTYPE_ISOCHRONUS) || (eptype == (USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF))) {
      _pma = USB_GetNextPMA(epsize);
      if(_pma == 0) return false;
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
      if(epsize & 0x1F) {
        epsize &= 0x1F;
      } else {
        epsize -= 0x20;
      }
      _rxcnt = 0x8000 | (epsize << 5);
      epsize += 0x20;
    } else {
      _rxcnt = epsize << 9;
    }
    _pma = USB_GetNextPMA(epsize);
    if(_pma == 0) return false;
    tbl->rx.addr = _pma;
    tbl->rx.cnt = _rxcnt;
    if((eptype == USB_EPTYPE_ISOCHRONUS) || (eptype == (USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF))) {
      _pma = USB_GetNextPMA(epsize);
      if(_pma == 0) return false;
      tbl->rx0.addr = _pma;
      tbl->rx0.cnt = _rxcnt;
      EP_DRX_UNSTALL(reg);
    } else {
      EP_RX_UNSTALL(reg);
    }
  }
  return true;
}
void         USB_DeconfigEP(uint8_t ep) {
  PMA_Table_t *ept = EPT(ep);
  *EPR(ep) &= ~USB_EPREG_MASK;
  ept->rx.addr = 0;
  ept->rx.cnt = 0;
  ept->tx.addr = 0;
  ept->tx.cnt = 0;
}
void         USB_RegisterEP(uint8_t ep, USB_Event_Callback_t callback) {
  USB_Device.Endpoint[ep & 0x07] = callback;
}
uint16_t     USB_ReadPMA(uint8_t *buf, uint16_t blen, PMA_Record_t *rx) {
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
int32_t      USB_ReadEP(uint8_t ep, void *buf, uint16_t blen) {
  PMA_Table_t *tbl = EPT(ep);
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
        return USB_ReadPMA(buf, blen, &(tbl->rx1));
      } else {
        return USB_ReadPMA(buf, blen, &(tbl->rx0));
      }
      /* isochronous endpoint */
    case (USB_EP_RX_VALID | USB_EP_ISOCHRONOUS):
      if (*reg & USB_EP_DTOG_RX) {
        return USB_ReadPMA(buf, blen, &(tbl->rx1));
      } else {
        return USB_ReadPMA(buf, blen, &(tbl->rx0));
      }
      /* regular endpoint */
    case (USB_EP_RX_NAK | USB_EP_BULK):
    case (USB_EP_RX_NAK | USB_EP_CONTROL):
    case (USB_EP_RX_NAK | USB_EP_INTERRUPT):
    {
      int32_t res = USB_ReadPMA(buf, blen, &(tbl->rx));
      /* setting endpoint to VALID state */
      EP_RX_VALID(reg);
      return res;
    }
      /* invalid or not ready */
    default:
      return -1;
  }
}
void         USB_WritePMA(uint8_t *buf, uint16_t blen, PMA_Record_t *tx) {
  uint16_t *pma = (void*) (USB_PMAADDR + tx->addr);
  tx->cnt = blen;
  while(blen > 1) {
    *pma++ = buf[1] << 8 | buf[0];
    buf += 2;
    blen -= 2;
  }
  if(blen) *pma = *buf;
}
int32_t      USB_WriteEP(uint8_t ep, void *buf, uint16_t blen) {
  PMA_Table_t *tbl = EPT(ep);
  volatile uint16_t *reg = EPR(ep);
  switch(*reg & (USB_EPTX_STAT | USB_EP_T_FIELD | USB_EP_KIND)) {
      /* doublebuffered bulk endpoint */
    case (USB_EP_TX_NAK | USB_EP_BULK | USB_EP_KIND):
      if(*reg & USB_EP_SWBUF_TX) {
        USB_WritePMA(buf, blen, &(tbl->tx1));
      } else {
        USB_WritePMA(buf, blen, &(tbl->tx0));
      }
      *reg = (*reg & USB_EPREG_MASK) | USB_EP_SWBUF_TX;
      break;
      /* isochronous endpoint */
    case(USB_EP_TX_VALID | USB_EP_ISOCHRONOUS):
      if (!(*reg & USB_EP_DTOG_TX)) {
        USB_WritePMA(buf, blen, &(tbl->tx1));
      } else {
        USB_WritePMA(buf, blen, &(tbl->tx0));
      }
      break;
      /* regular endpoint */
    case(USB_EP_TX_NAK | USB_EP_BULK):
    case(USB_EP_TX_NAK | USB_EP_CONTROL):
    case(USB_EP_TX_NAK | USB_EP_INTERRUPT):
      USB_WritePMA(buf, blen, &(tbl->tx));
      EP_TX_VALID(reg);
      break;
      /* invalid or not ready */
    default:
      return -1;
  }
  return blen;
}
void         USB_SetAddress(USB_ControlRequest_t *req) {
  USB->DADDR = USB_DADDR_EF | req->wValue;
  USB_Device.Status.device_state = (req->wValue) ? USB_STATE_ADDRESSED : USB_STATE_DEFAULT;
}
void         USB_StallEP0(uint8_t ep) {
  USB_StallEP(ep & 0x7F, 1);
  USB_StallEP(ep | 0x80, 1);
  USB_Device.Status.control_state = USB_CONTROL_IDLE;
}
void         USB_HandleEP0_Complete() {
  if(USB_Device.Complete_Callback) {
    USB_Device.Complete_Callback(USB_Device.Status.data_buf);
    USB_Device.Complete_Callback = 0;
  }
}
USB_Response USB_HandleEP0_Process(USB_ControlRequest_t *req) {
//  USB_Response r = USB_HandleEP0_Class(req, &(USB_Device.Complete_Callback));
//  if(r != USB_FAIL) return r;
  /* continuing standard USB requests */
  switch (req->bmRequestType & (USB_REQ_TYPE | USB_REQ_RECIPIENT)) {
    case USB_REQ_STANDARD | USB_REQ_DEVICE:
      switch (req->bRequest) {
        case USB_STD_CLEAR_FEATURE:
          return USB_FAIL;
        case USB_STD_GET_CONFIG:
          req->data[0] = USB_Device.Status.device_cfg;
          return USB_ACK;
        case USB_STD_GET_DESCRIPTOR:
          if((req->wValue >> 8) == USB_DTYPE_STRING) {
            if((req->wValue & 0xFF) == 0x00) {
              USB_Device.Status.data_ptr = (uint8_t*) StringLangID;
              USB_Device.Status.data_count = StringLangID[0];
              return USB_ACK;
            } else if ((req->wValue & 0xFF) == 0x01) {
              USB_Device.Status.data_ptr = (uint8_t*) StringVendor;
              USB_Device.Status.data_count = StringVendor[0];
              return USB_ACK;
            } else if ((req->wValue & 0xFF) == 0x02) {
              USB_Device.Status.data_ptr = (uint8_t*) StringProduct;
              USB_Device.Status.data_count = StringProduct[0];
              return USB_ACK;
            } else if ((req->wValue & 0xFF) == 0x03) {
              uint8_t *dsc = req->data;
              dsc[0] = 50;
              dsc[1] = USB_DTYPE_STRING;
              USB_IntToUnicode(*(__IO uint32_t*) (0x1FFFF7AC), &dsc[2]);
              USB_IntToUnicode(*(__IO uint32_t*) (0x1FFFF7B0), &dsc[18]);
              USB_IntToUnicode(*(__IO uint32_t*) (0x1FFFF7B4), &dsc[34]);
              USB_Device.Status.data_count = 50;
              return USB_ACK;
            } else {
              return USB_FAIL;
            }
          } else if ((req->wValue >> 8) == USB_DTYPE_DEVICE) {
            USB_Device.Status.data_ptr = (uint8_t*)USB_Descriptor;
            USB_Device.Status.data_count = USB_Descriptor[0];
            return USB_ACK;
          } else if ((req->wValue >> 8) == USB_DTYPE_CONFIGURATION) {
            USB_Device.Status.data_ptr = (uint8_t*)USB_ConfigDescriptor;
            USB_Device.Status.data_count = sizeof(USB_ConfigDescriptor);
            return USB_ACK;
          }
          return USB_FAIL;
        case USB_STD_GET_STATUS:
          req->data[0] = 0;
          req->data[1] = 0;
          return USB_ACK;
        case USB_STD_SET_ADDRESS:
          if(USB_GetInfo() & USBD_HW_ADDRFST) {
            USB_SetAddress(req);
          } else {
            USB_Device.Complete_Callback = USB_SetAddress;
          }
          return USB_ACK;
        case USB_STD_SET_CONFIG:
          if(USB_SetConfiguration(req->wValue) == USB_ACK) {
            USB_Device.Status.device_cfg = req->wValue;
            USB_Device.Status.device_state = (req->wValue) ? USB_STATE_CONFIGURED : USB_STATE_ADDRESSED;
            return USB_ACK;
          }
          return USB_FAIL;
        case USB_STD_SET_DESCRIPTOR:
          return USB_FAIL;
        case USB_STD_SET_FEATURE:
          return USB_FAIL;
      }
    case USB_REQ_STANDARD | USB_REQ_INTERFACE:
      switch (req->bRequest) {
        case USB_STD_GET_STATUS:
          req->data[0] = 0;
          req->data[1] = 0;
          return USB_ACK;
        default:
          return USB_FAIL;
      }
    case USB_REQ_STANDARD | USB_REQ_ENDPOINT:
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
          return USB_FAIL;
      }
    default:
      return USB_FAIL;
  }
}
void         USB_HandleEP0_TX() {
  int32_t _t;
  switch (USB_Device.Status.control_state) {
    case USB_CONTROL_TXPAYLOAD:
    case USB_CONTROL_TXDATA:
      _t = (USB_Device.Status.data_count < USB_Device.Status.ep0size) ? USB_Device.Status.data_count : USB_Device.Status.ep0size;
      USB_WriteEP(0, USB_Device.Status.data_ptr, _t);
      USB_Device.Status.data_ptr += _t;
      USB_Device.Status.data_count -= _t;
      /* if all data is not sent */
      if (0 != USB_Device.Status.data_count) break;
      /* if last packet has a EP0 size and host awaiting for the more data ZLP should be sent*/
      /* if ZLP required, control state will be unchanged, therefore next TX event sends ZLP */
      if (USB_CONTROL_TXDATA == USB_Device.Status.control_state || _t != USB_Device.Status.ep0size) {
        USB_Device.Status.control_state = USB_CONTROL_LASTDATA; /* no ZLP required */
      }
      break;
    case USB_CONTROL_LASTDATA:
      USB_Device.Status.control_state = USB_CONTROL_STATUSOUT;
      break;
    case USB_CONTROL_STATUSIN:
      USB_Device.Status.control_state = USB_CONTROL_IDLE;
      return USB_HandleEP0_Complete();
    default:
      /* unexpected TX completion */
      /* just skipping it */
      break;
  }
}
void         USB_HandleEP0_RX() {
  uint16_t _t;
  USB_ControlRequest_t * const req = USB_Device.Status.data_buf;
  switch (USB_Device.Status.control_state) {
    case USB_CONTROL_IDLE:
      /* read SETUP packet, send STALL_PID if incorrect packet length */
      if (0x08 != USB_ReadEP(0, req, USB_Device.Status.data_maxsize)) {
        return USB_StallEP0(0);
      }
      USB_Device.Status.data_ptr = req->data;
      USB_Device.Status.data_count = req->wLength;
      /* processing request with no payload data*/
      if ((req->bmRequestType & USB_REQ_DEVTOHOST) || (0 == req->wLength)) break;
      /* checking available memory for DATA OUT stage */
      if(req->wLength > USB_Device.Status.data_maxsize) return USB_StallEP0(0);
      /* continue DATA OUT stage */
      USB_Device.Status.control_state = USB_CONTROL_RXDATA;
      return;
    case USB_CONTROL_RXDATA:
      /*receive DATA OUT packet(s) */
      _t = USB_ReadEP(0, USB_Device.Status.data_ptr, USB_Device.Status.data_count);
      if (USB_Device.Status.data_count < _t) {
        /* if received packet is large than expected */
        /* Must be error. Let's drop this request */
        return USB_StallEP0(0);
      } else if (USB_Device.Status.data_count != _t) {
        /* if all data payload was not received yet */
        USB_Device.Status.data_count -= _t;
        USB_Device.Status.data_ptr += _t;
        return;
      }
      break;
    case USB_CONTROL_STATUSOUT:
      /* reading STATUS OUT data to buffer */
      USB_ReadEP(0, USB_Device.Status.data_ptr, USB_Device.Status.data_maxsize);
      USB_Device.Status.control_state = USB_CONTROL_IDLE;
      return USB_HandleEP0_Complete();
    default:
      /* unexpected RX packet */
      return USB_StallEP0(0);
  }
  /* usb request received. let's handle it */
  USB_Device.Status.data_ptr = req->data;
  USB_Device.Status.data_count = /*req->wLength;*/USB_Device.Status.data_maxsize;
  switch (USB_HandleEP0_Process(req)) {
    case USB_ACK:
      if (req->bmRequestType & USB_REQ_DEVTOHOST) {
        /* return data from function */
        if(USB_Device.Status.data_count >= req->wLength) {
          USB_Device.Status.data_count = req->wLength;
          USB_Device.Status.control_state = USB_CONTROL_TXDATA;
        } else {
          /* DATA IN packet smaller than requested */
          /* ZLP maybe wanted */
          USB_Device.Status.control_state = USB_CONTROL_TXPAYLOAD;
        }
        return USB_HandleEP0_TX(0x80);
      } else {
        /* confirming by ZLP in STATUS_IN stage */
        USB_WriteEP(0x80, 0, 0);
        USB_Device.Status.control_state = USB_CONTROL_STATUSIN;
      }
      break;
    case USB_NACK:
      USB_Device.Status.control_state = USB_CONTROL_STATUSIN;
      break;
    default:
      return USB_StallEP0(0);
  }
}
void         USB_HandleEP0(uint8_t event) {
  switch(event) {
    case usbd_evt_epsetup:
      /* force switch to setup state */
      USB_Device.Status.control_state = USB_CONTROL_IDLE;
      USB_Device.Complete_Callback = NULL;
    case USB_EVENT_RX:
      return USB_HandleEP0_RX();
    case USB_EVENT_TX:
      return USB_HandleEP0_TX();
    default:
      break;
  }
}
void         USB_ISR() {
  uint8_t _ev, _ep;
  uint16_t _istr = USB->ISTR;
  _ep = _istr & USB_ISTR_EP_ID;
  if(_istr & USB_ISTR_CTR) {
    volatile uint16_t *reg = EPR(_ep);
    if(*reg & USB_EP_CTR_TX) {
      *reg &= (USB_EPREG_MASK ^ USB_EP_CTR_TX);
      _ep |= 0x80;
      _ev = USB_EVENT_TX;
    } else {
      *reg &= (USB_EPREG_MASK ^ USB_EP_CTR_RX);
      _ev = (*reg & USB_EP_SETUP) ? usbd_evt_epsetup : USB_EVENT_RX;
    }
  } else if(_istr & USB_ISTR_RESET) {
    USB->ISTR &= ~USB_ISTR_RESET;
    USB->BTABLE = 0;
    for(int i = 0; i < 8; i++) USB_DeconfigEP(i);
    USB_Device.Status.device_state = USB_STATE_DEFAULT;
    USB_Device.Status.control_state = USB_CONTROL_IDLE;
    USB_Device.Status.device_cfg = 0;
    USB_ConfigEP(0, USB_EPTYPE_CONTROL, USB_Device.Status.ep0size);
    USB_Device.Endpoint[0] = USB_HandleEP0;
    USB->DADDR = USB_DADDR_EF;
    return;
//#if !defined(USBD_SOF_DISABLED)
//  } else if(_istr & USB_ISTR_SOF) {
//    _ev = usbd_evt_sof;
//    USB->ISTR &= ~USB_ISTR_SOF;
//#endif
  } else if(_istr & USB_ISTR_WKUP) {
    _ev = usbd_evt_wkup;
    USB->CNTR &= ~USB_CNTR_FSUSP;
    USB->ISTR &= ~USB_ISTR_WKUP;
  } else if (_istr & USB_ISTR_SUSP) {
//    USB_Device.Status.device_state = USB_STATE_DEFAULT;
    _ev = usbd_evt_susp;
    USB->CNTR |= USB_CNTR_FSUSP;
    USB->ISTR &= ~USB_ISTR_SUSP;
    return;
  } else if (_istr & USB_ISTR_ERR) {
    USB->ISTR &= ~USB_ISTR_ERR;
    _ev = usbd_evt_error;
  } else {
    return;
  }
  switch(_ev) {
    case USB_EVENT_RX:
    case USB_EVENT_TX:
    case usbd_evt_epsetup:
      if(USB_Device.Endpoint[_ep & 0x07]) USB_Device.Endpoint[_ep & 0x07](_ev);
      break;
    default:
      break;
  }
//  if(udev.Events[_ev]) udev.Events[_ev](_ev, _ep);


//  usbd_process_evt(&udev, _ev, _ep);
}