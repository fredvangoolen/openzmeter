#ifndef __USB_CDC_H
#define __USB_CDC_H
//#include "stm32f37x_it.h"
#include "stm32f37x.h"
#include "usb_lib.h"
//#include "usb_pwr.h"
#include <usb_type.h>
#include <stdbool.h>
//Descriptor defines ---------------------------------------------------------------------------------------------------
#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define VIRTUAL_COM_PORT_DATA_SIZE                64
#define VIRTUAL_COM_PORT_INT_SIZE                 64

#define VIRTUAL_COM_PORT_SIZ_DEVICE_DESC          18
#define VIRTUAL_COM_PORT_SIZ_CONFIG_DESC          67
#define VIRTUAL_COM_PORT_SIZ_STRING_LANGID         4
#define VIRTUAL_COM_PORT_SIZ_STRING_VENDOR        10
#define VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT       82
#define VIRTUAL_COM_PORT_SIZ_STRING_SERIAL        50

#define SET_COMM_FEATURE                        0x02
#define SET_LINE_CODING                         0x20
#define GET_LINE_CODING                         0x21
#define SET_CONTROL_LINE_STATE                  0x22
//Packet buffer area definition ----------------------------------------------------------------------------------------
#define BTABLE_ADDRESS      (0x000)                 /* Table start */
#define ENDP0_RXADDR        (BTABLE_ADDRESS + 32)   /* EP0 - OUT */
#define ENDP0_TXADDR        (ENDP0_RXADDR   + 64)   /* EP0 - IN */
#define ENDP1_TXADDR        (ENDP0_TXADDR   + 64)   /* EP1 - IN (CDC_DATA) */
#define ENDP2_TXADDR        (ENDP1_TXADDR   + 64)   /* EP2 - IN (CDC_ACM) */
#define ENDP3_RXADDR        (ENDP2_TXADDR   + 64)   /* EP3 - OUT (CDC_DATA) */
//CDC structures -------------------------------------------------------------------------------------------------------
typedef struct {
  uint32_t bitrate;
  uint8_t format;
  uint8_t paritytype;
  uint8_t datatype;
} LINE_CODING;
//USB and CDC enums ----------------------------------------------------------------------------------------------------
typedef enum _RESUME_STATE {
  RESUME_EXTERNAL,
  RESUME_INTERNAL,
  RESUME_LATER,
  RESUME_WAIT,
  RESUME_START,
  RESUME_ON,
  RESUME_OFF,
  RESUME_ESOF
} RESUME_STATE;

typedef enum _DEVICE_STATE {
  UNCONNECTED,
  ATTACHED,
  POWERED,
  SUSPENDED,
  ADDRESSED,
  CONFIGURED
} DEVICE_STATE;
//Exported variables ---------------------------------------------------------------------------------------------------
//extern __IO bool fPortOpen;
//CDC class specific ---------------------------------------------------------------------------------------------------
void     CDC_Init(void);
void     CDC_Reset(void);
void     CDC_SetConfiguration(void);
void     CDC_SetDeviceAddress (void);
void     CDC_StatusIN(void);
void     CDC_StatusOUT(void);
RESULT   CDC_DataSetup(uint8_t);
RESULT   CDC_NoDataSetup(uint8_t);
RESULT   CDC_GetInterfaceSetting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t *CDC_GetDeviceDescriptor(uint16_t );
uint8_t *CDC_GetConfigDescriptor(uint16_t);
uint8_t *CDC_GetStringDescriptor(uint16_t);
uint8_t *CDC_GetLineCoding(uint16_t Length);
uint8_t *CDC_SetLineCoding(uint16_t Length);
//USB application functions --------------------------------------------------------------------------------------------
void     USB_LP_CAN1_RX0_IRQHandler(void);

void   Suspend(void);
void   Resume_Init(void);
void   Resume(RESUME_STATE eResumeSetVal);
RESULT PowerOn(void);
RESULT PowerOff(void);

void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config(FunctionalState NewState);
void USART_Config_Default(void);
bool USART_Config(void);
void OnUsbDataRx(uint8_t* data_buffer, uint8_t Nb_bytes);
void UsbSendData(const unsigned char* data, int len);
//void Get_SerialNum(void);
void IntToUnicode(uint32_t value, uint8_t *pbuf);
#endif  /*__HW_CONFIG_H*/

