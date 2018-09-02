#include "USB_CDC.h"

const uint8_t CDC_DeviceDescriptor[VIRTUAL_COM_PORT_SIZ_DEVICE_DESC] = {
  0x12,                       /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE, /* bDescriptorType */
  0x00,
  0x02,                       /* bcdUSB = 2.00 */
  0x00,                       /* bDeviceClass: Defined at interface level */
  0x00,                       /* bDeviceSubClass */
  0x00,                       /* bDeviceProtocol */
  0x40,                       /* bMaxPacketSize0 */
  0x83,
  0x04,                       /* idVendor */
  0x70,
  0x72,                       /* idProduct */
  0x00,
  0x02,                       /* bcdDevice = 2.00 */
  1,                          /* Index of string descriptor describing manufacturer */
  2,                          /* Index of string descriptor describing product */
  3,                          /* Index of string descriptor describing the device's serial number */
  0x01                        /* bNumConfigurations */
};

const uint8_t CDC_ConfigDescriptor[VIRTUAL_COM_PORT_SIZ_CONFIG_DESC] = {
  /*Configuation Descriptor*/
  0x09,   /* bLength: Configuation Descriptor size */
  USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType: Configuration */
  VIRTUAL_COM_PORT_SIZ_CONFIG_DESC,       /* wTotalLength:no of returned bytes */
  0x00,
  0x02,   /* bNumInterfaces: 2 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x00,   /* MaxPower 0 mA */

  /*Interface Descriptor*/
  0x09,   /* bLength: Interface Descriptor size */
  USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x01,   /* bNumEndpoints: One endpoints used */
  0x02,   /* bInterfaceClass: Communication Interface Class */
  0x02,   /* bInterfaceSubClass: Abstract Control Model */
  0x00,   /* bInterfaceProtocol: No protocol */
  0x00,   /* iInterface: */
  
  /* Header Functional Descriptor */
  0x05,   /* bLength: Endpoint Descriptor size */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x00,   /* bDescriptorSubtype: Header Func Desc */
  0x10,   /* bcdCDC: spec release number */
  0x01,

  /* Call Management Functional Descriptor */
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
  0x03,   /* bmCapabilities: D0+D1 */
  0x01,   /* bDataInterface: 1 */

  /* ACM Functional Descriptor */
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */
  
  /* Union Functional Descriptor */
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0x00,   /* bMasterInterface: Communication class interface */
  0x01,   /* bSlaveInterface0: Data Class Interface */

  /* Endpoint 2 Descriptor */
  0x07,                         /* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: Endpoint */
  0x82,                         /* bEndpointAddress */
  0x03,                         /* bmAttributes: Interrupt */
  VIRTUAL_COM_PORT_INT_SIZE,    /* wMaxPacketSize: */
  0x00,
  0x80,                         /* bInterval: */
  
  /*Data class interface descriptor*/
  0x09,   /* bLength: Endpoint Descriptor size */
  USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
  0x01,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0x0A,   /* bInterfaceClass: CDC */
  0x00,   /* bInterfaceSubClass: */
  0x00,   /* bInterfaceProtocol: */
  0x00,   /* iInterface: */
  
  /*Endpoint 3 Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
  0x03,   /* bEndpointAddress: (OUT3) */
  0x02,   /* bmAttributes: Bulk */
  VIRTUAL_COM_PORT_DATA_SIZE,      /* wMaxPacketSize: */
  0x00,
  0x00,   /* bInterval: ignore for Bulk transfer */
  
  /*Endpoint 1 Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
  0x81,   /* bEndpointAddress: (IN1) */
  0x02,   /* bmAttributes: Bulk */
  VIRTUAL_COM_PORT_DATA_SIZE,      /* wMaxPacketSize: */
  0x00,
  0x00,   /* bInterval: ignore for Bulk transfer */
};


/* USB String Descriptors */
const uint8_t CDC_StringLangID[VIRTUAL_COM_PORT_SIZ_STRING_LANGID] = {
  VIRTUAL_COM_PORT_SIZ_STRING_LANGID,
  USB_STRING_DESCRIPTOR_TYPE,
  0x0A,
  0x04 /* LangID = 0x040A: Spanish */
};

const uint8_t CDC_StringVendor[VIRTUAL_COM_PORT_SIZ_STRING_VENDOR] = {
  VIRTUAL_COM_PORT_SIZ_STRING_VENDOR, /* Size of Vendor string */
  USB_STRING_DESCRIPTOR_TYPE,         /* bDescriptorType*/
  'z', 0, 'R', 0, 'e', 0, 'd', 0
};

const uint8_t CDC_StringProduct[VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT] = {
  VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT, /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,          /* bDescriptorType */
  'z', 0, 'M', 0, 'e', 0, 't', 0, 'e', 0, 'r', 0, ' ', 0, '-', 0, 
  ' ', 0, 'A', 0, 'n', 0, 'a', 0, 'l', 0, 'i', 0, 'z', 0, 'a', 0,
  'd', 0, 'o', 0, 'r', 0, ' ', 0, 'd', 0, 'e', 0, ' ', 0, 'c', 0, 
  'a', 0, 'l', 0, 'i', 0, 'd', 0, 'a', 0, 'd', 0, ' ', 0, 'e', 0, 
  'l', 0, 'e', 0, 'c', 0, 't', 0, 'r', 0, 'i', 0, 'c', 0, 'a', 0
};

uint8_t CDC_StringSerial[VIRTUAL_COM_PORT_SIZ_STRING_SERIAL] = {
  VIRTUAL_COM_PORT_SIZ_STRING_SERIAL, /* bLength */
  USB_STRING_DESCRIPTOR_TYPE /* bDescriptorType */
};
//----------------------------------------------------------------------------------------------------------------------
LINE_CODING linecoding = {
  115200, /* baud rate*/
  0x00,   /* stop bits-1*/
  0x00,   /* parity - none*/
  0x08    /* no. of bits 8*/
};

DEVICE Device_Table = { 
  4,   /* Number of endpoints that are used */
  1    /* Number of configuration available */
};

DEVICE_PROP Device_Property = { 
  CDC_Init, CDC_Reset, CDC_StatusIN, CDC_StatusOUT, CDC_DataSetup, CDC_NoDataSetup, CDC_GetInterfaceSetting, 
  CDC_GetDeviceDescriptor, CDC_GetConfigDescriptor, CDC_GetStringDescriptor, 0, 0x40
};

USER_STANDARD_REQUESTS User_Standard_Requests = {
  NOP_Process,
  CDC_SetConfiguration,
  NOP_Process,
  NOP_Process,
  NOP_Process,
  NOP_Process,
  NOP_Process,
  NOP_Process,
  CDC_SetDeviceAddress
};

ONE_DESCRIPTOR Device_Descriptor = {
  (uint8_t *)CDC_DeviceDescriptor,
  VIRTUAL_COM_PORT_SIZ_DEVICE_DESC
};

ONE_DESCRIPTOR Config_Descriptor = {
  (uint8_t *)CDC_ConfigDescriptor,
  VIRTUAL_COM_PORT_SIZ_CONFIG_DESC
};

ONE_DESCRIPTOR String_Descriptor[4] = {
  { (uint8_t *)CDC_StringLangID,  VIRTUAL_COM_PORT_SIZ_STRING_LANGID },
  { (uint8_t *)CDC_StringVendor,  VIRTUAL_COM_PORT_SIZ_STRING_VENDOR },
  { (uint8_t *)CDC_StringProduct, VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT },
  { (uint8_t *)CDC_StringSerial,  VIRTUAL_COM_PORT_SIZ_STRING_SERIAL }
};

uint8_t       Request         = 0;
__IO uint32_t bDeviceState    = UNCONNECTED; /* USB device status */
__IO bool     fSuspendEnabled = false;       /* true when suspend is possible */
//__IO bool     fPortOpen       = false;       /* true when port is open (DTR=1) */
/*******************************************************************************
 * Function Name  : CDC_Init.
 * Description    : Virtual COM Port init routine.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void CDC_Init(void) {
  IntToUnicode(*(__IO uint32_t*)(0x1FFFF7AC), &CDC_StringSerial[2]);  
  IntToUnicode(*(__IO uint32_t*)(0x1FFFF7B0), &CDC_StringSerial[18]);
  IntToUnicode(*(__IO uint32_t*)(0x1FFFF7B4), &CDC_StringSerial[34]);
  
  pInformation->Current_Configuration = 0;
  PowerOn();
  USB_SIL_Init();
  bDeviceState = UNCONNECTED;
}

/*******************************************************************************
 * Function Name  : CDC_Reset
 * Description    : Virtual_Com_Port reset routine
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void CDC_Reset(void) {
  pInformation->Current_Configuration = 0;
  pInformation->Current_Feature = CDC_ConfigDescriptor[7];
  pInformation->Current_Interface = 0;

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
  SetEPTxAddr(ENDP1, ENDP1_TXADDR);
  SetEPTxStatus(ENDP1, EP_TX_NAK);
  SetEPRxStatus(ENDP1, EP_RX_DIS);

  /* Initialize Endpoint 2 */
  SetEPType(ENDP2, EP_INTERRUPT);
  SetEPTxAddr(ENDP2, ENDP2_TXADDR);
  SetEPRxStatus(ENDP2, EP_RX_DIS);
  SetEPTxStatus(ENDP2, EP_TX_NAK);

  /* Initialize Endpoint 3 */
  SetEPType(ENDP3, EP_BULK);
  SetEPRxAddr(ENDP3, ENDP3_RXADDR);
  SetEPRxCount(ENDP3, VIRTUAL_COM_PORT_DATA_SIZE);
  SetEPRxStatus(ENDP3, EP_RX_VALID);
  SetEPTxStatus(ENDP3, EP_TX_DIS);
  
  SetDeviceAddress(0);
  bDeviceState = ATTACHED;
}

/*******************************************************************************
 * Function Name  : CDC_SetConfiguration.
 * Description    : Update the device state to configured.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void CDC_SetConfiguration(void) {
  DEVICE_INFO *pInfo = &Device_Info;
  if(pInfo->Current_Configuration != 0) bDeviceState = CONFIGURED;
}

/*******************************************************************************
 * Function Name  : CDC_SetDeviceAddress.
 * Description    : Update the device state to addressed.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void CDC_SetDeviceAddress(void) {
  bDeviceState = ADDRESSED;
}

/*******************************************************************************
 * Function Name  : CDC_StatusIN.
 * Description    : Virtual COM Port Status In Routine.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void CDC_StatusIN(void) {
  if(Request == SET_LINE_CODING) {
    Request = 0;
  }
}

/*******************************************************************************
 * Function Name  : CDC_StatusOUT
 * Description    : Virtual COM Port Status OUT Routine.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void CDC_StatusOUT(void) {
}

/*******************************************************************************
 * Function Name  : CDC_DataSetup
 * Description    : handle the data class specific requests
 * Input          : Request Nb.
 * Output         : None.
 * Return         : USB_UNSUPPORT or USB_SUCCESS.
 *******************************************************************************/
RESULT CDC_DataSetup(uint8_t RequestNo) {
  uint8_t * (*CopyRoutine)(uint16_t);
  CopyRoutine = NULL;

  if (RequestNo == GET_LINE_CODING) {
    if(Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) CopyRoutine = CDC_GetLineCoding;
  } else if(RequestNo == SET_LINE_CODING) {
    if(Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) CopyRoutine = CDC_SetLineCoding;
    Request = SET_LINE_CODING;
  } 
  if(CopyRoutine == NULL) return USB_UNSUPPORT;

  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);
  return USB_SUCCESS;
}

/*******************************************************************************
 * Function Name  : CDC_NoDataSetup.
 * Description    : handle the no data class specific requests.
 * Input          : Request Nb.
 * Output         : None.
 * Return         : USB_UNSUPPORT or USB_SUCCESS.
 *******************************************************************************/
RESULT CDC_NoDataSetup(uint8_t RequestNo) {
  if(Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) {
    if(RequestNo == SET_COMM_FEATURE) return USB_SUCCESS;
    if(RequestNo == SET_CONTROL_LINE_STATE) {
//      fPortOpen = ((pInformation->USBwValues.bw.bb0 & 0x01) == 0x01);
      return USB_SUCCESS;
    }
  }
  return USB_UNSUPPORT;
}

/*******************************************************************************
 * Function Name  : CDC_GetDeviceDescriptor.
 * Description    : Gets the device descriptor.
 * Input          : Length.
 * Output         : None.
 * Return         : The address of the device descriptor.
 *******************************************************************************/
uint8_t *CDC_GetDeviceDescriptor(uint16_t Length) {
  return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

/*******************************************************************************
 * Function Name  : CDC_GetConfigDescriptor.
 * Description    : get the configuration descriptor.
 * Input          : Length.
 * Output         : None.
 * Return         : The address of the configuration descriptor.
 *******************************************************************************/
uint8_t *CDC_GetConfigDescriptor(uint16_t Length) {
  return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/*******************************************************************************
 * Function Name  : CDC_GetStringDescriptor
 * Description    : Gets the string descriptors according to the needed index
 * Input          : Length.
 * Output         : None.
 * Return         : The address of the string descriptors.
 *******************************************************************************/
uint8_t *CDC_GetStringDescriptor(uint16_t Length) {
  uint8_t wValue0 = pInformation->USBwValue0;
  if(wValue0 > 4) return NULL;
  return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
}

/*******************************************************************************
 * Function Name  : CDC_GetInterfaceSetting.
 * Description    : test the interface and the alternate setting according to the
 *                  supported one.
 * Input1         : uint8_t: Interface : interface number.
 * Input2         : uint8_t: AlternateSetting : Alternate Setting number.
 * Output         : None.
 * Return         : The address of the string descriptors.
 *******************************************************************************/
RESULT CDC_GetInterfaceSetting(uint8_t Interface, uint8_t AlternateSetting) {
  if (AlternateSetting > 0) return USB_UNSUPPORT;
  if (Interface > 1) return USB_UNSUPPORT;
  return USB_SUCCESS;
}

/*******************************************************************************
 * Function Name  : CDC_GetLineCoding.
 * Description    : send the linecoding structure to the PC host.
 * Input          : Length.
 * Output         : None.
 * Return         : Linecoding structure base address.
 *******************************************************************************/
uint8_t *CDC_GetLineCoding(uint16_t Length) {
  if(Length == 0) {
    pInformation->Ctrl_Info.Usb_wLength = sizeof (linecoding);
    return NULL;
  }
  return (uint8_t *)&linecoding;
}

/*******************************************************************************
 * Function Name  : CDC_SetLineCoding.
 * Description    : Set the linecoding structure fields.
 * Input          : Length.
 * Output         : None.
 * Return         : Linecoding structure base address.
 *******************************************************************************/
uint8_t *CDC_SetLineCoding(uint16_t Length) {
  if(Length == 0) {
    pInformation->Ctrl_Info.Usb_wLength = sizeof (linecoding);
    return NULL;
  }
  return (uint8_t *)&linecoding;
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











//ErrorStatus HSEStartUpStatus;
//USART_InitTypeDef USART_InitStructure;

//uint8_t USART_Rx_Buffer[USART_RX_DATA_SIZE];
//uint32_t USART_Rx_ptr_in = 0;
//uint32_t USART_Rx_ptr_out = 0;
//uint32_t USART_Rx_length = 0;

//static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len);

//#define ADC1_DR_Address    ((uint32_t)0x4001244C)




