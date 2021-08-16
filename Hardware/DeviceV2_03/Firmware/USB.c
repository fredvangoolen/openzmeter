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

#include "Main.h"
#include "USB.h"
// Device descriptor ---------------------------------------------------------------------------------------------------
const uint8_t USB_Descriptor[] = {
  0x12,                                                                         //bLength
  USB_DTYPE_DEVICE,                                                             //bDescriptorType
  0x00, 0x02,                                                                   //bcdUSB = 2.00
  0xFF,                                                                         //bDeviceClass: vendor_defined
  0x00,                                                                         //bDeviceSubClass
  0x00,                                                                         //bDeviceProtocol
  0x40,                                                                         //bMaxPacketSize0
  0x83, 0x04,                                                                   //idVendor
  0x70, 0x72,                                                                   //idProduct
  0x02, 0x03,                                                                   //bcdDevice = 2.03
  1,                                                                            //Index of string descriptor describing manufacturer
  2,                                                                            //Index of string descriptor describing product
  3,                                                                            //Index of string descriptor describing the device's serial number
  0x01                                                                          //bNumConfigurations
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
const uint8_t StringProduct[] = {                                               //Product name string
  0x38,                                                                         //bLength
  USB_DTYPE_STRING,                                                             //bDescriptorType
  'o', 0, 'p', 0, 'e', 0, 'n', 0, 'Z', 0, 'm', 0, 'e', 0, 't', 0,               //
  'e', 0, 'r', 0, ' ', 0, '-', 0, ' ', 0, 'C', 0, 'a', 0, 'p', 0,               //
  't', 0, 'u', 0, 'r', 0, 'e', 0, ' ', 0, 'd', 0, 'e', 0, 'v', 0,               //
  'i', 0, 'c', 0, 'e', 0                                                        //
};                                                                              //
// Access macros -------------------------------------------------------------------------------------------------------
#define USB_PMASIZE   0x200                                                     //
#define EPR     ((volatile uint32_t *)USB_BASE)                                 //
#define EPT     ((PMA_Table_t *)USB_PMAADDR)                                    //
// Types ---------------------------------------------------------------------------------------------------------------
typedef struct {                                                                //PMA record struct
  uint32_t     Addr;                                                            //
  uint32_t     Size;                                                            //
} PMA_Record_t;                                                                 //
typedef struct {                                                                //PMA table struct
  PMA_Record_t TX;                                                              //
  PMA_Record_t RX;                                                              //
} PMA_Table_t;                                                                  //
typedef struct {                                                                //Control request struct
  uint8_t      Type;                                                            //
  uint8_t      Request;                                                         //
  uint16_t     Value;                                                           //
  uint16_t     Index;                                                           //
  uint16_t     Length;                                                          //
  uint8_t      Data[56];                                                        //
} USB_ControlRequest_t;                                                         //
typedef void (*USB_Event_Callback_t)(const uint8_t event);                      //Used callbacks
typedef void (*USB_Request_Callback_t)();                                       //
// Variables -----------------------------------------------------------------------------------------------------------
static USB_ControlRequest_t   CtrReq          __attribute__((section(".ccm"))); //Active control transfer (if any)
static USB_Request_Callback_t OnComplete      __attribute__((section(".ccm"))); //Control transfer complete callback
static USB_Event_Callback_t   Endpoint[8]     __attribute__((section(".ccm"))); //Callbacks of each endpoint
static uint8_t               *DataPtr         __attribute__((section(".ccm"))); //Pointer to current data for control request
static uint16_t               DataCount       __attribute__((section(".ccm"))); //Count remained data for control request
static uint8_t                DeviceCfg       __attribute__((section(".ccm"))); //Current device configuration number
static uint8_t                DeviceState     __attribute__((section(".ccm"))); //Current state machine state
static uint8_t                ControlState    __attribute__((section(".ccm"))); //Current control transfer state
// Functions -----------------------------------------------------------------------------------------------------------
void USB_IntToUnicode(uint32_t value, uint8_t *pbuf) {                          //Format serial number to buffer
  for(uint8_t i = 0; i < 8; i++, value = value >> 4) {                          //
    *(pbuf++) = (value & 0x0F) + (((value & 0x0F) < 0x0A) ? '0' : 'A' - 10);    //
    *(pbuf++) = 0;                                                              //
    value = value >> 4;                                                         //
  }                                                                             //
}                                                                               //
void USB_StallEP(const uint8_t ep, const bool stall) {                          //Stall endpoint
  uint8_t EP = ep & 0x07;                                                       //
  if(USB_EP_ISOCHRONOUS == (EPR[EP] & USB_EP_T_FIELD)) return;                  //ignore ISOCHRONOUS endpoints
  if(ep & USB_EPDIR_OUT) {                                                      //IN endpoint
    if(USB_EP_TX_DIS == (EPR[EP] & USB_EPTX_STAT)) return;                      //DISABLED endpoint can't be stalled or unstalled
    if(stall) {                                                                 //
      EPR[EP] = (EPR[EP] ^ USB_EP_TX_STALL) & (USB_EPREG_MASK | USB_EPTX_STAT); //
    } else {                                                                    //
      if((USB_EP_KIND|USB_EP_BULK) == (EPR[EP]&(USB_EP_T_FIELD|USB_EP_KIND))) { //doublebuffered endpoint
        EPR[EP] = (EPR[EP] ^ USB_EP_TX_VALID) &                                 //set endpoint to VALID and clear DTOG_TX & SWBUF_TX
            (USB_EPREG_MASK | USB_EPTX_STAT | USB_EP_DTOG_TX | USB_EP_SWBUF_TX);//
      } else {                                                                  //
        EPR[EP] = (EPR[EP] ^ USB_EP_TX_NAK) &                                   //
            (USB_EPREG_MASK | USB_EPTX_STAT | USB_EP_DTOG_TX);                  //
      }                                                                         //
    }                                                                           //
  } else {                                                                      //
    if(USB_EP_RX_DIS == (EPR[EP] & USB_EPRX_STAT)) return;                      //DISABLED endpoint can't be stalled or unstalled
    if(stall) {                                                                 //
      EPR[EP] = (EPR[EP] ^ USB_EP_RX_STALL) & (USB_EPREG_MASK | USB_EPRX_STAT); //
    } else {                                                                    //
      if((USB_EP_KIND|USB_EP_BULK) == (EPR[EP]&(USB_EP_T_FIELD|USB_EP_KIND))) { //doublebuffered endpoint
        EPR[EP] = (EPR[EP] ^ (USB_EP_RX_VALID | USB_EP_SWBUF_RX)) &             //set endpoint to VALID, clear DTOG_RX, set SWBUF_RX
            (USB_EPREG_MASK | USB_EPRX_STAT | USB_EP_DTOG_RX | USB_EP_SWBUF_RX);//
      } else {                                                                  //
        EPR[EP] = (EPR[EP] ^ USB_EP_RX_VALID) &                                 //set endpoint to VALID and clear DTOG_RX
            (USB_EPREG_MASK | USB_EPRX_STAT | USB_EP_DTOG_RX);                  //
      }                                                                         //
    }                                                                           //
  }                                                                             //
}                                                                               //
void USB_RegisterEP(uint8_t ep, USB_Event_Callback_t callback) {                //Set endpoint event callback
  Endpoint[ep & 0x07] = callback;                                               //
}                                                                               //
void USB_SetAddress() {                                                         //Set device address
  USB->DADDR = USB_DADDR_EF | CtrReq.Value;                                     //
  DeviceState = (CtrReq.Value) ? USB_STATE_ADDRESSED : USB_STATE_DEFAULT;       //
}                                                                               //
void USB_WritePMA(uint8_t *buf, uint16_t blen, PMA_Record_t *tx) {              //Write buffer to PMA
  uint16_t *Dest = (uint16_t*)(USB_PMAADDR + 2 * tx->Addr);                     //
  tx->Size = blen;                                                              //
  for(uint16_t n = 0, Word = 0; n < blen; n++, buf++) {                         //
    Word |= (*buf) << ((n & 0x01) ? 8 : 0);                                     //
    if((n & 0x01) || ((n + 1) == blen)) {                                       //
      *Dest = Word;                                                             //
      Dest += 2;                                                                //
      Word = 0;                                                                 //
    }                                                                           //
  }                                                                             //
}                                                                               //
uint16_t USB_ReadPMA(uint8_t *buf, uint16_t blen, PMA_Record_t *rx) {           //Fill buffer from PMA
  uint16_t *Src =  (uint16_t*)(USB_PMAADDR + 2 * rx->Addr);                     //
  blen = ((rx->Size & 0x03FF) < blen) ? (rx->Size & 0x03FF) : blen;             //
  for(uint16_t n = 0, Word = 0; n < blen; n++, buf++) {                         //
    if(n & 0x01) {                                                              //
      *buf = Word >> 8;                                                         //
    } else {                                                                    //
      Word = *Src;                                                              //
      Src += 2;                                                                 //
      *buf = Word & 0xFF;                                                       //
    }                                                                           //
  }                                                                             //
  return blen;                                                                  //
}                                                                               //
int16_t USB_WriteEP(uint8_t ep, uint8_t *buf, uint16_t blen) {                  //Send data on endpoint
  uint8_t EP = ep & 0x07;                                                       //
  switch(EPR[EP] & (USB_EPTX_STAT | USB_EP_T_FIELD | USB_EP_KIND)) {            //
    case (USB_EP_TX_NAK | USB_EP_BULK | USB_EP_KIND) :                          //doublebuffered bulk endpoint
      if(EPR[EP] & USB_EP_SWBUF_TX) {                                           //
        USB_WritePMA(buf, blen, &(EPT[EP].RX));                                 //
      } else {                                                                  //
        USB_WritePMA(buf, blen, &(EPT[ep].TX));                                 //
      }                                                                         //
      EPR[EP] = (EPR[EP] & USB_EPREG_MASK) | USB_EP_SWBUF_TX;                   //
      break;                                                                    //
    case (USB_EP_TX_VALID | USB_EP_ISOCHRONOUS) :                               //isochronous endpoint
      if((EPR[EP] & USB_EP_DTOG_TX) == 0) {                                     //
        USB_WritePMA(buf, blen, &(EPT[EP].RX));                                 //
      } else {                                                                  //
        USB_WritePMA(buf, blen, &(EPT[ep].TX));                                 //
      }                                                                         //
      break;                                                                    //
    case (USB_EP_TX_NAK | USB_EP_BULK) :                                        //regular endpoint
    case (USB_EP_TX_NAK | USB_EP_CONTROL) :                                     //
    case (USB_EP_TX_NAK | USB_EP_INTERRUPT) :                                   //
      USB_WritePMA(buf, blen, &(EPT[EP].TX));                                   //
      EPR[EP] = (EPR[EP] ^ USB_EP_TX_VALID) & (USB_EPREG_MASK | USB_EPTX_STAT); //
      break;                                                                    //
    default :                                                                   //invalid or not ready
      return -1;                                                                //
  }                                                                             //
  return blen;                                                                  //
}                                                                               //
int16_t USB_ReadEP(uint8_t ep, uint8_t *buf, uint16_t blen) {                   //Read data from endpoint
  int32_t Read = 0;                                                             //
  uint8_t EP = ep & 0x07;                                                       //
  switch(EPR[EP] & (USB_EPRX_STAT | USB_EP_T_FIELD | USB_EP_KIND)) {            //
    case (USB_EP_RX_VALID | USB_EP_BULK | USB_EP_KIND) :                        //doublebuffered bulk endpoint
      switch(EPR[EP] & (USB_EP_DTOG_RX | USB_EP_SWBUF_RX)) {                    //switching SWBUF if EP is NAKED
        case 0 :                                                                //
        case (USB_EP_DTOG_RX | USB_EP_SWBUF_RX) :                               //
          EPR[EP] = (EPR[EP] & USB_EPREG_MASK) | USB_EP_SWBUF_RX;               //
          break;                                                                //
        default:                                                                //
          break;                                                                //
      }                                                                         //
      if(EPR[EP] & USB_EP_SWBUF_RX) {                                           //
        return USB_ReadPMA(buf, blen, &(EPT[EP].RX));                           //
      } else {                                                                  //
        return USB_ReadPMA(buf, blen, &(EPT[EP].TX));                           //
      }                                                                         //
    case (USB_EP_RX_VALID | USB_EP_ISOCHRONOUS) :                               //isochronous endpoint
      if(EPR[EP] & USB_EP_DTOG_RX) {                                            //
        return USB_ReadPMA(buf, blen, &(EPT[EP].RX));                           //
      } else {                                                                  //
        return USB_ReadPMA(buf, blen, &(EPT[EP].TX));                           //
      }                                                                         //
    case (USB_EP_RX_NAK | USB_EP_BULK) :                                        //regular endpoint
    case (USB_EP_RX_NAK | USB_EP_CONTROL) :                                     //
    case (USB_EP_RX_NAK | USB_EP_INTERRUPT) :                                   //
      Read = USB_ReadPMA(buf, blen, &(EPT[EP].RX));                             //
      EPR[EP] = (EPR[EP] ^ USB_EP_RX_VALID) & (USB_EPREG_MASK | USB_EPRX_STAT); //setting endpoint to VALID state
      return Read;                                                              //
    default:                                                                    //invalid or not ready
      return -1;                                                                //
  }                                                                             //
}
USB_Response USB_HandleEP0_Process() {                                          //Handle EP0 requests
  uint8_t Type = CtrReq.Type & (USB_REQ_TYPE | USB_REQ_RECIPIENT);              //
  if(Type == (USB_REQ_STANDARD|USB_REQ_DEVICE)) {                               //
    if(CtrReq.Request == USB_STD_GET_STATUS) {                                  //Standard-Device-GetStatus request
      CtrReq.Data[0] = 0;                                                       //
      CtrReq.Data[1] = 0;                                                       //
      return USB_ACK;                                                           //
    } else if(CtrReq.Request == USB_STD_GET_CONFIG) {                           //Standard-Device-GetConfig request
      CtrReq.Data[0] = DeviceCfg;                                               //
      return USB_ACK;                                                           //
    } else if(CtrReq.Request == USB_STD_SET_ADDRESS) {                          //Standard-Device-SetAddress request
      OnComplete = USB_SetAddress;                                              //
      return USB_ACK;                                                           //
    } else if(CtrReq.Request == USB_STD_GET_DESCRIPTOR) {                       //Standard-Device-GetDescriptor request
      if((CtrReq.Value >> 8) == USB_DTYPE_STRING) {                             //
        if((CtrReq.Value & 0xFF) == 0x00) {                                     //
          DataPtr = (uint8_t*) StringLangID;                                    //
          DataCount = StringLangID[0];                                          //
          return USB_ACK;                                                       //
        } else if ((CtrReq.Value & 0xFF) == 0x01) {                             //
          DataPtr = (uint8_t*) StringVendor;                                    //
          DataCount = StringVendor[0];                                          //
          return USB_ACK;                                                       //
        } else if ((CtrReq.Value & 0xFF) == 0x02) {                             //
          DataPtr = (uint8_t*) StringProduct;                                   //
          DataCount = StringProduct[0];                                         //
          return USB_ACK;                                                       //
        } else if ((CtrReq.Value & 0xFF) == 0x03) {                             //
          uint8_t *dsc = CtrReq.Data;                                           //
          dsc[0] = 50;                                                          //
          dsc[1] = USB_DTYPE_STRING;                                            //
          USB_IntToUnicode(*(__IO uint32_t*) (0x1FFFF7AC), &dsc[2]);            //
          USB_IntToUnicode(*(__IO uint32_t*) (0x1FFFF7B0), &dsc[18]);           //
          USB_IntToUnicode(*(__IO uint32_t*) (0x1FFFF7B4), &dsc[34]);           //
          DataCount = 50;                                                       //
          return USB_ACK;                                                       //
        }                                                                       //
      } else if((CtrReq.Value >> 8) == USB_DTYPE_DEVICE) {                      //
        DataPtr = (uint8_t*)USB_Descriptor;                                     //
        DataCount = USB_Descriptor[0];                                          //
        return USB_ACK;                                                         //
      } else if ((CtrReq.Value >> 8) == USB_DTYPE_CONFIGURATION) {              //
        DataPtr = (uint8_t*)USB_ConfigDescriptor;                               //
        DataCount = sizeof(USB_ConfigDescriptor);                               //
        return USB_ACK;                                                         //
      }                                                                         //
    } else if(CtrReq.Request == USB_STD_SET_CONFIG) {                           //Standard-Device-SetConfig request
      if(CtrReq.Value == 0) {
//      USB_DeconfigEP(SEND_EP);
//      USB_RegisterEP(SEND_EP, 0);
//      USB_DeconfigEP(RECV_EP);
//      USB_RegisterEP(RECV_EP, 0);
        DeviceState = USB_STATE_ADDRESSED;
        DeviceCfg = CtrReq.Value;                                               //
      return USB_ACK;                                                           //
      } else if(CtrReq.Value == 1) {                                            //
//      USB_ConfigEP(SEND_EP, USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF, 64);
//      USB_RegisterEP(SEND_EP, USB_HandleEP1_TX);
//      USB_ConfigEP(RECV_EP, USB_EPTYPE_BULK, 2);
//      USB_RegisterEP(RECV_EP, USB_HandleEP2_RX);
        DeviceState = USB_STATE_CONFIGURED;
        DeviceCfg = CtrReq.Value;                                               //
        return USB_ACK;                                                         //
      }                                                                         //
    }                                                                           //
  } else if(Type == (USB_REQ_STANDARD | USB_REQ_INTERFACE)) {                   //
    if(CtrReq.Request == USB_STD_GET_STATUS) {                                  //Standard-Interface-GetStatus
      CtrReq.Data[0] = 0;                                                       //
      CtrReq.Data[1] = 0;                                                       //
      return USB_ACK;                                                           //
    }                                                                           //
  } else if(Type == (USB_REQ_STANDARD | USB_REQ_ENDPOINT)) {                    //
    if(CtrReq.Request == USB_STD_SET_FEATURE) {                                 //Standard-Endpoint-SetFeature
      USB_StallEP(CtrReq.Index, 1);                                             //
      return USB_ACK;                                                           //
    } else if(CtrReq.Request == USB_STD_CLEAR_FEATURE) {                        //Standard-Endpoint-ClearFeature
      USB_StallEP(CtrReq.Index, 0);                                             //
      return USB_ACK;                                                           //
    } else if(CtrReq.Request == USB_STD_GET_STATUS) {                           //Standard-Endpoint-GetStatus
      CtrReq.Data[0] = 0;                                                       //
      if(CtrReq.Index & USB_EPDIR_OUT) {                                        //
        if(USB_EPTX_STAT & EPR[CtrReq.Index & 0x07]) CtrReq.Data[0] = 1;        //
      } else {                                                                  //
        if(USB_EPRX_STAT & EPR[CtrReq.Index & 0x07]) CtrReq.Data[0] = 1;        //
      }                                                                         //
      CtrReq.Data[1] = 0;                                                       //
      return USB_ACK;                                                           //
    }                                                                           //
  }                                                                             //
  return USB_FAIL;                                                              //
}                                                                               //
void USB_StallEP0() {                                                           //Stall EP0
  USB_StallEP(0x00, true);                                                      //
  USB_StallEP(0x80, true);                                                      //
  ControlState = USB_CONTROL_IDLE;                                              //
}                                                                               //
void USB_HandleEP0_TX() {                                                       //Handle TX control transfers
  int16_t Write = 0;                                                            //
  switch(ControlState) {                                                        //
    case USB_CONTROL_TXPAYLOAD:                                                 //
    case USB_CONTROL_TXDATA:                                                    //
      Write = (DataCount < sizeof(CtrReq)) ? DataCount : sizeof(CtrReq);        //
      USB_WriteEP(0, DataPtr, Write);                                           //
      DataPtr += Write;                                                         //
      DataCount -= Write;                                                       //
      if(DataCount != 0) return;                                                //if all data is not sent
      if((ControlState == USB_CONTROL_TXDATA) || (sizeof(CtrReq) != Write)) {   //If no ZLP needed
        ControlState = USB_CONTROL_LASTDATA;                                    //
      }                                                                         //
      return;                                                                   //
    case USB_CONTROL_LASTDATA:                                                  //On ZLP needed do nothing
      ControlState = USB_CONTROL_STATUSOUT;                                     //
      return;                                                                   //
    case USB_CONTROL_STATUSIN:                                                  //Terminate transaction
      ControlState = USB_CONTROL_IDLE;                                          //
      if(OnComplete) OnComplete();                                              //Call complete callback
      return;                                                                   //
  }                                                                             //
}                                                                               //
void USB_HandleEP0_RX() {                                                       //Handle RX control transfers
  uint16_t Read = 0;                                                            //
  switch(ControlState) {                                                        //
    case USB_CONTROL_IDLE :                                                     //read SETUP packet
      if(USB_ReadEP(0, (uint8_t*)&CtrReq, sizeof(CtrReq.Data)) != 0x08) {       //Stall if incorrect packet length
        USB_StallEP0();                                                         //
        return;                                                                 //
      }                                                                         //
      DataPtr = CtrReq.Data;                                                    //Init reception pointer
      DataCount = CtrReq.Length;                                                //
      if((CtrReq.Type & USB_REQ_DEVTOHOST) || (CtrReq.Length == 0)) break;      //if no payload data
      if(CtrReq.Length > sizeof(CtrReq.Data)) return USB_StallEP0(0);           //check available memory for DATA OUT
      ControlState = USB_CONTROL_RXDATA;                                        //continue DATA OUT stage
      return;                                                                   //
    case USB_CONTROL_RXDATA :                                                   //receive DATA OUT packet(s)
      Read = USB_ReadEP(0, DataPtr, DataCount);                                 //Get received data
      if(Read > DataCount) {                                                    //If more data than expected
        return USB_StallEP0();                                                  //
      } else if(DataCount != Read) {                                            //if all data payload was not received yet
        DataCount -= Read;                                                      //
        DataPtr += Read;                                                        //
        return;                                                                 //
      }                                                                         //
      break;                                                                    //
    case USB_CONTROL_STATUSOUT :                                                //reading STATUS OUT data to buffer
      USB_ReadEP(0, 0, 0);                                                      //
      ControlState = USB_CONTROL_IDLE;                                          //
      if(OnComplete) OnComplete();                                              //Call complete callback
      return;                                                                   //
    default  :                                                                  //unexpected RX packet
      return USB_StallEP0();                                                    //
  }                                                                             //
  DataPtr = CtrReq.Data;                                                        //Handle received data
  DataCount = sizeof(CtrReq.Data);                                              //
  switch(USB_HandleEP0_Process()) {                                             //
    case USB_ACK :                                                              //On success
      if(CtrReq.Type & USB_REQ_DEVTOHOST) {                                     //
        if(DataCount >= CtrReq.Length) {                                        //return data from function
          DataCount = CtrReq.Length;                                            //
          ControlState = USB_CONTROL_TXDATA;                                    //
        } else {                                                                //
          ControlState = USB_CONTROL_TXPAYLOAD;                                 //
        }                                                                       //
        USB_HandleEP0_TX();                                                     //
      } else {                                                                  //
        USB_WriteEP(0x80, 0, 0);                                                //confirming by ZLP in STATUS_IN stage
        ControlState = USB_CONTROL_STATUSIN;                                    //
      }                                                                         //
      break;                                                                    //
    case USB_NACK :                                                             //
      ControlState = USB_CONTROL_STATUSIN;                                      //
      break;                                                                    //
    default :                                                                   //
      return USB_StallEP0();                                                    //
  }                                                                             //
}                                                                               //
void USB_HandleEP0(const uint8_t evt) {                                         //Handle EP0 CONTROL transfers
  if(evt == USB_EVENT_TX) {                                                     //
    USB_HandleEP0_TX();                                                         //
  } else {                                                                      //
    if(evt == USB_EVENT_SETUP) {                                                //
      ControlState = USB_CONTROL_IDLE;                                          //
      OnComplete = NULL;                                                        //
    }                                                                           //
    USB_HandleEP0_RX();                                                         //
  }                                                                             //
}                                                                               //
uint16_t USB_GetNextPMA(const uint16_t sz) {                                    //Get PMA address for new endpoints
  uint16_t Addr = USB_PMASIZE;                                                  //
  for(uint8_t i = 0; i < 8; i++) {                                              //
    if((EPT[i].RX.Addr) && (EPT[i].RX.Addr < Addr)) Addr = EPT[i].RX.Addr;      //
    if((EPT[i].TX.Addr) && (EPT[i].TX.Addr < Addr)) Addr = EPT[i].TX.Addr;      //
  }                                                                             //
  return Addr - sz;                                                             //
}                                                                               //
void USB_ConfigEP(const uint8_t ep, const uint8_t type, uint16_t size) {        //Configure endpoint
  uint8_t EP = (ep & 0x07);                                                     //
  if(type == USB_EPTYPE_CONTROL) {                                              //Set type and number
    EPR[EP] = USB_EP_CONTROL | EP;                                              //
  } else if(type == USB_EPTYPE_BULK) {                                          //
    EPR[EP] = USB_EP_BULK | EP;                                                 //
  } else if(type == (USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF)) {                    //
    EPR[EP] = USB_EP_BULK | USB_EP_KIND | EP;                                   //
  } else {                                                                      //
    EPR[EP] = USB_EP_INTERRUPT | EP;                                            //
  }                                                                             //
  size = (~0x01U) & (size + 1);                                                 //Force 2-byte aligned size
  if(((ep & USB_EPDIR_OUT) != 0) || (type == USB_EPTYPE_CONTROL)) {             //Set IN endpoints address
    EPT[EP].TX.Addr = USB_GetNextPMA(size);                                     //
    EPT[EP].TX.Size  = 0;                                                       //
    if(type == (USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF)) {                         //
      EPT[EP].RX.Addr = USB_GetNextPMA(size);                                   //
      EPT[EP].RX.Size  = 0;                                                     //
      EPR[EP] = (EPR[EP] ^ USB_EP_TX_VALID) &                                   //
           (USB_EPREG_MASK | USB_EPTX_STAT | USB_EP_DTOG_TX | USB_EP_SWBUF_TX); //
    } else {                                                                    //
      EPR[EP] = (EPR[EP] ^ USB_EP_TX_NAK) &                                     //
           (USB_EPREG_MASK | USB_EPTX_STAT | USB_EP_DTOG_TX);                   //
    }                                                                           //
  }                                                                             //
  if((ep & USB_EPDIR_OUT) == 0) {                                               //Set OUT endpoints address
    if(size > 62) {                                                             //
      size = (~0x1FU) & (size + 0x1FU);                                         //On larger than 62bytes force 32-byte aligned address
      EPT[EP].RX.Size = 0x8000U - 0x400U + (size << 5);                         //
    } else {                                                                    //
      EPT[EP].RX.Size = size << 9;                                              //
    }                                                                           //
    EPT[EP].RX.Addr = USB_GetNextPMA(size);                                     //
    if((type == (USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF))) {                       //
      EPT[EP].TX.Addr = USB_GetNextPMA(size);                                   //
      EPT[EP].TX.Size  = EPT[EP].RX.Size;                                       //
      EPR[EP] = (EPR[EP] ^ (USB_EP_RX_VALID | USB_EP_SWBUF_RX)) &               //
           (USB_EPREG_MASK | USB_EPRX_STAT | USB_EP_DTOG_RX | USB_EP_SWBUF_RX); //
    } else {                                                                    //
      EPR[EP] = (EPR[EP] ^ USB_EP_RX_VALID) &                                   //
           (USB_EPREG_MASK | USB_EPRX_STAT | USB_EP_DTOG_RX);                   //
    }                                                                           //
  }                                                                             //
}                                                                               //
void USB_DeconfigEP(const uint8_t ep) {                                         //Deconfigure endpoint
  uint8_t EP = ep & 0x07;                                                       //
  EPR[EP] &= ~USB_EPREG_MASK;                                                   //
  EPT[EP].RX.Addr = 0;                                                          //
  EPT[EP].RX.Size = 0;                                                          //
  EPT[EP].TX.Addr = 0;                                                          //
  EPT[EP].TX.Size = 0;                                                          //
}                                                                               //
void USB_LP_CAN_RX0_ISR() {                                                     //USB interrupt handler
  if(USB->ISTR & USB_ISTR_CTR) {                                                //Manage correct transfer complete
    uint8_t EP = USB->ISTR & 0x07;                                              //
    if(EPR[EP] & USB_EP_CTR_TX) {                                               //
      EPR[EP] &= (USB_EPREG_MASK ^ USB_EP_CTR_TX);                              //
      if(Endpoint[EP]) Endpoint[EP](USB_EVENT_TX);                              //
    } else {                                                                    //
      EPR[EP] &= (USB_EPREG_MASK ^ USB_EP_CTR_RX);                              //
      if(Endpoint[EP]) {                                                        //
        Endpoint[EP]((EPR[EP] & USB_EP_SETUP) ? USB_EVENT_SETUP : USB_EVENT_RX);//
      }                                                                         //
    }                                                                           //
  } else if(USB->ISTR & USB_ISTR_RESET) {                                       //Manage bus reset
    USB->ISTR &= ~USB_ISTR_RESET;                                               //
    USB->BTABLE = 0;                                                            //
    for(uint8_t i = 0; i < 8; i++) USB_DeconfigEP(i);                           //
    DeviceState = USB_STATE_DEFAULT;                                            //
    ControlState = USB_CONTROL_IDLE;                                            //
    DeviceCfg = 0;                                                              //
    USB_ConfigEP(0, USB_EPTYPE_CONTROL, sizeof(CtrReq));                        //
    Endpoint[0] = USB_HandleEP0;                                                //
    USB->DADDR = USB_DADDR_EF;                                                  //
  } else if(USB->ISTR & USB_ISTR_WKUP) {                                        //Manage bus wake up
    USB->CNTR &= ~USB_CNTR_FSUSP;                                               //
    USB->ISTR &= ~USB_ISTR_WKUP;                                                //
  } else if (USB->ISTR & USB_ISTR_SUSP) {                                       //Manage bus suspend
    USB->CNTR |= USB_CNTR_FSUSP;                                                //
    USB->ISTR &= ~USB_ISTR_SUSP;                                                //
  } else if (USB->ISTR & USB_ISTR_ERR) {                                        //Manage bus error
    USB->ISTR &= ~USB_ISTR_ERR;                                                 //
  }                                                                             //
}                                                                               //
void USB_Init() {                                                               //Init USB port
  RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_USBEN;                        //Enable clock to USB peripheral
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;                                            //
  GPIOA->MODER |= GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1 |                 //Configure ports
                   GPIO_MODER_MODER8_0;                                         //
  GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR11_Msk | GPIO_OSPEEDER_OSPEEDR12_Msk;  //High speed mode
  GPIOA->AFR[1] |= (14 << GPIO_AFRH_AFRH3_Pos);                                 //Set port A (Pin 11) to alternate function USB
  GPIOA->AFR[1] |= (14 << GPIO_AFRH_AFRH4_Pos);                                 //Set port A (Pin 12) to alternate function USB
  USB->CNTR = USB_CNTR_CTRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_ERRM | //Enabled interrupts
              USB_CNTR_RESETM;                                                  //
  GPIOA->BSRR |= GPIO_BSRR_BS_8;                                                //Enable resistor
  DataPtr = (uint8_t*)&CtrReq;                                                  //Configure control request buffer
  NVIC_SetPriority(USB_LP_CAN_RX0_IRQn, 1);                                     //Enable interrupts
  NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);                                          //
}                                                                               //
// ---------------------------------------------------------------------------------------------------------------------