/* ================================ openZmeter ================================
 * File:     Main.h
 * Author:   Eduardo Viciana (2017)
 * ---------------------------------------------------------------------------- */
#pragma once
#include "stm32f30x.h"
#include "usb_type.h"


//Configuracion del firmware -------------------------------------------------------------------------------------------
#define INBUFFER_LEN   1024

//Funciones del ADC ----------------------------------------------------------------------------------------------------
void ADC_Configure();
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

