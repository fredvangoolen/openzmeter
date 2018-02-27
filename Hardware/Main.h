/* ================================ openZmeter ================================
 * File:     Main.h
 * Author:   Eduardo Viciana (2017)
 * ---------------------------------------------------------------------------- */
#pragma once
#include <inttypes.h>
#include "stm32f30x.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_adc.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_misc.h"
#include "usb_type.h"
#include "usb_lib.h"


//Configuracion del firmware -------------------------------------------------------------------------------------------
#define INBUFFER_LEN   1024

//Variables exportadas -------------------------------------------------------------------------------------------------
extern uint64_t Sequence;
extern uint16_t INBuff[];
extern uint16_t INReadPos;
extern uint16_t INWritePos;
extern uint16_t INBuffUsed;
//extern uint8_t  ChannelMask;
//Funciones del buffer -------------------------------------------------------------------------------------------------
void ResetBuffers();
void AppendBuffer(uint16_t sample);
//Funciones del ADC ----------------------------------------------------------------------------------------------------
void ADC_Configure();
void ADC_StartCapture(uint8_t channels, uint8_t mode);
void ADC_StopCapture();
//Funciones del USB ----------------------------------------------------------------------------------------------------
void USB_Configure();


/* Exported functions ------------------------------------------------------- */
//void Set_System(void);
//void Set_USBClock(void);
//void GPIO_AINConfig(void);
//void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
//void USB_Interrupts_Config(void);
void USB_Cable_Config (FunctionalState NewState);
//void Joystick_Send(uint8_t Keys);
//uint8_t JoyState(void);
//void Get_SerialNum(void);

