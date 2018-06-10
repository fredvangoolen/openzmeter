/* ================================ openZmeter ================================
 * File:     Main.h
 * Author:   Eduardo Viciana (2017)
 * ---------------------------------------------------------------------------- */
#pragma once
#include "stm32f30x.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_adc.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_misc.h"
#include "usb_type.h"
#include "usb_lib.h"
#include <inttypes.h>
//Configuracion del firmware ---------------------------------------------------
#define INBUFFER_LEN   8192
//Variables exportadas ---------------------------------------------------------
//Funciones generales ----------------------------------------------------------
void Waitms(uint32_t ms);
//Funciones del buffer ---------------------------------------------------------
//void Buffers_Init(uint8_t channels);
void Buffer_AppendSamples();
bool Buffers_ToPMA(uint16_t dst);
void Buffers_Reset();
//Modulo ADC  ------------------------------------------------------------------
extern float Offsets[7];
void ADC_Configure();
bool ADC_StartCapture(uint8_t channels, uint8_t decimation, uint16_t speed);
void ADC_StopCapture();
//Funciones del USB ------------------------------------------------------------
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

