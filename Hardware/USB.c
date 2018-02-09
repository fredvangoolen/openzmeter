#include "USB.h"
//Descriptores USB -----------------------------------------------------------------------------------------------------
const uint8_t CDC_DeviceDescriptor[DEVICE_DESC_SIZE] = {
  0x12,                                   /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE,             /* bDescriptorType */
  0x00, 0x02,                             /* bcdUSB = 2.00 */
  0xFF,                                   /* bDeviceClass: vendor_defined */
  0x00,                                   /* bDeviceSubClass */
  0x00,                                   /* bDeviceProtocol */
  0x40,                                   /* bMaxPacketSize0 */
  0x83, 0x04,                             /* idVendor */
  0x70, 0x72,                             /* idProduct */
  0x00, 0x02,                             /* bcdDevice = 2.00 */
  1,                                      /* Index of string descriptor describing manufacturer */
  2,                                      /* Index of string descriptor describing product */
  3,                                      /* Index of string descriptor describing the device's serial number */
  0x01                                    /* bNumConfigurations */
};
const uint8_t CDC_ConfigDescriptor[CONFIG_DESC_SIZE] = {
  0x09,                                   /* bLength: Configuation Descriptor size */
  USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType: Configuration */
  CONFIG_DESC_SIZE, 0x00,                 /* wTotalLength:no of returned bytes */
  0x01,                                   /* bNumInterfaces: 1 interface */
  0x01,                                   /* bConfigurationValue: Configuration value */
  0x00,                                   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,                                   /* bmAttributes: self powered */
  0x00,                                   /* MaxPower 0 mA */

  0x09,                                   /* bLength: Interface Descriptor size */
  USB_INTERFACE_DESCRIPTOR_TYPE,          /* bDescriptorType: Interface */
  0x00,                                   /* bInterfaceNumber: Number of Interface */
  0x00,                                   /* bAlternateSetting: Alternate setting */
  0x01,                                   /* bNumEndpoints: One endpoints used */
  0xFF,                                   /* bInterfaceClass: Communication Interface Class */
  0x00,                                   /* bInterfaceSubClass: Abstract Control Model */
  0x00,                                   /* bInterfaceProtocol: No protocol */
  0x00,                                   /* iInterface: */
  
  0x07,                                   /* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,           /* bDescriptorType: Endpoint */
  0x81,                                   /* bEndpointAddress: (IN1) */
  0x02,                                   /* bmAttributes: Bulk */
  IN_DATA_SIZE, 0x00,                     /* wMaxPacketSize: */
  0x00,                                   /* bInterval: ignore for Bulk transfer */
};
const uint8_t StringLangID[STRING_LANGID_SIZE] = { 
  STRING_LANGID_SIZE, 
  USB_STRING_DESCRIPTOR_TYPE, 
  0x0A, 0x04                              /* LangID = 0x040A: Spanish */
};
const uint8_t StringVendor[STRING_VENDOR_SIZE] = {
  STRING_VENDOR_SIZE,                     /* Size of Vendor string */
  USB_STRING_DESCRIPTOR_TYPE,             /* bDescriptorType*/
  'z', 0, 'R', 0, 'e', 0, 'd', 0
};
const uint8_t StringProduct[STRING_PRODUCT_SIZE] = {
  STRING_PRODUCT_SIZE,                     /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,              /* bDescriptorType */
  'o', 0, 'p', 0, 'e', 0, 'n', 0, 'Z', 0, 'm', 0, 'e', 0, 't', 0, 
  'e', 0, 'r', 0, ' ', 0, '-', 0, ' ', 0, 'C', 0, 'a', 0, 'a', 0,
  'p', 0, 't', 0, 'u', 0, 'r', 0, 'e', 0, ' ', 0, 'd', 0, 'e', 0, 
  'v', 0, 'i', 0, 'c', 0, 'e', 0 
};
uint8_t StringSerial[STRING_SERIAL_SIZE] = {
  STRING_SERIAL_SIZE,                     /* bLength */
  USB_STRING_DESCRIPTOR_TYPE              /* bDescriptorType */
};
//Estructuras del driver USB -------------------------------------------------------------------------------------------
ONE_DESCRIPTOR Device_Descriptor = {
  (uint8_t *)CDC_DeviceDescriptor, DEVICE_DESC_SIZE
};
ONE_DESCRIPTOR Config_Descriptor = {
  (uint8_t *)CDC_ConfigDescriptor, CONFIG_DESC_SIZE
};
ONE_DESCRIPTOR String_Descriptor[4] = {
  { (uint8_t *)StringLangID,  STRING_LANGID_SIZE },
  { (uint8_t *)StringVendor,  STRING_VENDOR_SIZE },
  { (uint8_t *)StringProduct, STRING_PRODUCT_SIZE },
  { (uint8_t *)StringSerial,  STRING_SERIAL_SIZE }
};
DEVICE Device_Table = { 
  2,                                      /* Number of endpoints that are used */
  1                                       /* Number of configuration available */
};
__IO uint32_t bDeviceState = UNCONNECTED; /* USB device status */
//DEVICE_PROP ----------------------------------------------------------------------------------------------------------
void USB_Class_Init(void) {
  IntToUnicode(*(__IO uint32_t*)(0x1FFFF7E8), &StringSerial[2]);  
  IntToUnicode(*(__IO uint32_t*)(0x1FFFF7EC), &StringSerial[18]);
  IntToUnicode(*(__IO uint32_t*)(0x1FFFF7F0), &StringSerial[34]);
  pInformation->Current_Configuration = 0;
  PowerOn();
  USB_SIL_Init();
  bDeviceState = UNCONNECTED;
}
void USB_Class_Reset(void) {
  pInformation->Current_Configuration = 0;
  pInformation->Current_Feature       = CDC_ConfigDescriptor[7];
  pInformation->Current_Interface     = 0;
  SetBTABLE(BTABLE_ADDRESS);

  /* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_NAK);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxValid(ENDP0);

  /* Initialize Endpoint 1 */
  SetEPType(ENDP1, EP_BULK);
  SetEPDoubleBuff(ENDP1);
  SetEPDblBuffAddr(ENDP1, ENDP1_BUF0ADDR, ENDP1_BUF1ADDR);
  SetEPDblBuffCount(ENDP1, EP_DBUF_IN, Device_Property.MaxPacketSize);

  ClearDTOG_TX(ENDP1);  // USB PERIPHERAL
  ClearDTOG_RX(ENDP1);  // SW_BUF for APPLICATION
  ToggleDTOG_RX(ENDP1); // NOT TX ie SW_BUF <- this is actually set which buffer will be first for usb periph and which first for software/firmware
  SetEPTxStatus(ENDP1, EP_TX_NAK);
  SetEPRxStatus(ENDP1, EP_RX_DIS);

  SetDeviceAddress(0);
  bDeviceState = ATTACHED;
}
RESULT USB_Class_DataSetup(uint8_t RequestNo) {
//  uint8_t * (*CopyRoutine)(uint16_t);
//  CopyRoutine = NULL;

//  if (RequestNo == GET_LINE_CODING) {
//    if(Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) CopyRoutine = CDC_GetLineCoding;
//  } else if(RequestNo == SET_LINE_CODING) {
//    if(Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) CopyRoutine = CDC_SetLineCoding;
//    Request = SET_LINE_CODING;
//  } 
//  if(CopyRoutine == NULL) 
  return USB_UNSUPPORT;

//  pInformation->Ctrl_Info.CopyData = CopyRoutine;
//  pInformation->Ctrl_Info.Usb_wOffset = 0;
//  (*CopyRoutine)(0);
//  return USB_SUCCESS;
}
RESULT USB_Class_NoDataSetup(uint8_t RequestNo) {
  return USB_UNSUPPORT;
}
RESULT USB_Class_GetInterfaceSetting(uint8_t Interface, uint8_t AlternateSetting) {
  if((AlternateSetting != 0) || (Interface != 1)) return USB_UNSUPPORT;
  return USB_SUCCESS;
}
uint8_t *USB_Class_GetDeviceDescriptor(uint16_t Length) {
  return Standard_GetDescriptorData(Length, &Device_Descriptor);
}
uint8_t *USB_Class_GetConfigDescriptor(uint16_t Length) {
  return Standard_GetDescriptorData(Length, &Config_Descriptor);
}
uint8_t *USB_Class_GetStringDescriptor(uint16_t Length) {
  uint8_t wValue0 = pInformation->USBwValue0;
  if(wValue0 > 4) return NULL;
  return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
}
DEVICE_PROP Device_Property = { 
  USB_Class_Init, 
  USB_Class_Reset, 
  NOP_Process, 
  NOP_Process, 
  USB_Class_DataSetup, 
  USB_Class_NoDataSetup, 
  USB_Class_GetInterfaceSetting, 
  USB_Class_GetDeviceDescriptor, 
  USB_Class_GetConfigDescriptor, 
  USB_Class_GetStringDescriptor, 
  0, 0x40
};
//USER_STANDARD_REQUESTS -----------------------------------------------------------------------------------------------
void USB_Class_SetConfiguration(void) {
  DEVICE_INFO *pInfo = &Device_Info;
  if(pInfo->Current_Configuration != 0) bDeviceState = CONFIGURED;
}
void USB_Class_SetDeviceAddress(void) {
  bDeviceState = ADDRESSED;
}
USER_STANDARD_REQUESTS User_Standard_Requests = {
  NOP_Process,
  USB_Class_SetConfiguration,
  NOP_Process,
  NOP_Process,
  NOP_Process,
  NOP_Process,
  NOP_Process,
  NOP_Process,
  USB_Class_SetDeviceAddress
};
//----------------------------------------------------------------------------------------------------------------------
uint8_t       Request         = 0;

__IO bool     fSuspendEnabled = false;       /* true when suspend is possible */







/*******************************************************************************
 * Function Name  : CDC_GetConfigDescriptor.
 * Description    : get the configuration descriptor.
 * Input          : Length.
 * Output         : None.
 * Return         : The address of the configuration descriptor.
 *******************************************************************************/


/*******************************************************************************
 * Function Name  : CDC_GetStringDescriptor
 * Description    : Gets the string descriptors according to the needed index
 * Input          : Length.
 * Output         : None.
 * Return         : The address of the string descriptors.
 *******************************************************************************/




/*******************************************************************************
 * Function Name  : CDC_GetLineCoding.
 * Description    : send the linecoding structure to the PC host.
 * Input          : Length.
 * Output         : None.
 * Return         : Linecoding structure base address.
 *******************************************************************************/
uint8_t *CDC_GetLineCoding(uint16_t Length) {
//  if(Length == 0) {
//    pInformation->Ctrl_Info.Usb_wLength = sizeof (linecoding);
//    return NULL;
//  }
//  return (uint8_t *)&linecoding;
}

/*******************************************************************************
 * Function Name  : CDC_SetLineCoding.
 * Description    : Set the linecoding structure fields.
 * Input          : Length.
 * Output         : None.
 * Return         : Linecoding structure base address.
 *******************************************************************************/
uint8_t *CDC_SetLineCoding(uint16_t Length) {
//  if(Length == 0) {
//    pInformation->Ctrl_Info.Usb_wLength = sizeof (linecoding);
//    return NULL;
//  }
//  return (uint8_t *)&linecoding;
}

/*******************************************************************************
 * Description    : Restores system clocks and power while exiting suspend mode
 *******************************************************************************/
void Leave_LowPowerMode(void) {
  DEVICE_INFO *pInfo = &Device_Info;

  /* Set the device state to the correct state */
  if (pInfo->Current_Configuration != 0) {
    /* Device configured */
    bDeviceState = CONFIGURED;
  } else {
    bDeviceState = ATTACHED;
  }
}

/*******************************************************************************
 * Description    : Software Connection/Disconnection of USB Cable
 *******************************************************************************/
void USB_Cable_Config(FunctionalState NewState) {
}

/*******************************************************************************
 * Function Name  : PowerOn
 * Description    :
 * Input          : None.
 * Output         : None.
 * Return         : USB_SUCCESS.
 *******************************************************************************/
RESULT PowerOn(void) {
#ifndef STM32F10X_CL
  uint16_t wRegVal;

  /*** cable plugged-in ? ***/
  USB_Cable_Config(ENABLE);

  /*** CNTR_PWDN = 0 ***/
  wRegVal = CNTR_FRES;
  _SetCNTR(wRegVal);

  /*** CNTR_FRES = 0 ***/
  wInterrupt_Mask = 0;
  _SetCNTR(wInterrupt_Mask);
  /*** Clear pending interrupts ***/
  _SetISTR(0);
  /*** Set interrupt mask ***/
  wInterrupt_Mask = CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;
  _SetCNTR(wInterrupt_Mask);
#endif /* STM32F10X_CL */

  return USB_SUCCESS;
}
void IntToUnicode(uint32_t value, uint8_t *pbuf) {
  for(uint8_t i = 0; i < 8; i++) {
    *(pbuf++) = (value & 0x0F) + (((value & 0x0F) < 0x0A) ? '0' : 'A' - 10);
    *(pbuf++) = 0;
    value = value >> 4;
  }
}
