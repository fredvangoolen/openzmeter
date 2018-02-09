#ifndef __USB_CDC_H
#define __USB_CDC_H
#include "stm32f10x_it.h"
#include "usb_lib.h"
#include "USB.h"
#include "usb_pwr.h"
#include <usb_type.h>
#include <stdbool.h>
//Descriptor defines ---------------------------------------------------------------------------------------------------
#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define IN_DATA_SIZE                              64
#define DEVICE_DESC_SIZE                          18
#define CONFIG_DESC_SIZE                          25
#define STRING_LANGID_SIZE                         4
#define STRING_VENDOR_SIZE                        10
#define STRING_PRODUCT_SIZE                       58
#define STRING_SERIAL_SIZE                        50
//Packet buffer area definition ----------------------------------------------------------------------------------------
#define BTABLE_ADDRESS                       (0x000)                 /* Table start */
#define ENDP0_RXADDR           (BTABLE_ADDRESS + 32)   /* EP0 - OUT */
#define ENDP0_TXADDR           (ENDP0_RXADDR   + 64)   /* EP0 - IN */
#define ENDP1_BUF0ADDR         (ENDP0_TXADDR   + 64)   /* EP1 - IN (Buff1) */
#define ENDP1_BUF1ADDR       (ENDP1_BUF0ADDR   + 64)   /* EP1 - IN (Buff2) */

//USB and CDC enums ----------------------------------------------------------------------------------------------------
typedef enum RESUME_STATE { RESUME_EXTERNAL, RESUME_INTERNAL, RESUME_LATER, RESUME_WAIT, RESUME_START, RESUME_ON, RESUME_OFF, RESUME_ESOF } RESUME_STATE;
typedef enum _DEVICE_STATE { UNCONNECTED, ATTACHED, POWERED, SUSPENDED, ADDRESSED, CONFIGURED } DEVICE_STATE;

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

