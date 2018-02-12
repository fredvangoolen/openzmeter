/* ================================ openZmeter ================================
 * File:     ADC_Capture.h
 * Author:   Eduardo Viciana (2017)
 * ---------------------------------------------------------------------------- */
#include <stdint.h>
#include "stm32f30x_rcc.h"
#include "stm32f30x_adc.h"
#include "stm32f30x_gpio.h"

//Tipos y funciones externas ---------------------------------------------------
typedef struct {
  int16_t VoltageA;
  int16_t CurrentA;
} ADCSample_ADC1;
typedef struct {
  int16_t VoltageB;
  int16_t CurrentB;
} ADCSample_ADC2;
typedef struct {
  int16_t VoltageC;
  int16_t CurrentC;
} ADCSample_ADC3;
typedef struct {
  int16_t CurrentD;
} ADCSample_ADC4;
extern void AppendSample(int16_t value);

// Buffers de captura para el DMA ----------------------------------------------
ADCSample_ADC1 ADC1_Buffer[32];     //El maximo oversampling que se permite es 16x
ADCSample_ADC2 ADC2_Buffer[32];     //El maximo oversampling que se permite es 16x
ADCSample_ADC2 ADC3_Buffer[32];     //El maximo oversampling que se permite es 16x
ADCSample_ADC2 ADC4_Buffer[32];     //El maximo oversampling que se permite es 16x

/* Calibra el ADC*/
void ADC_InitADC(ADC_TypeDef* ADCx) {
  ADC_InitTypeDef ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_VoltageRegulatorCmd(ADCx, ENABLE);
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_Circular;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;
  ADC_CommonInit(ADCx, &ADC_CommonInitStructure);
  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_RisingEdge;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
  ADC_InitStructure.ADC_NbrOfRegChannel = 1;
  ADC_Init(ADCx, &ADC_InitStructure);
  ADC_SelectCalibrationMode(ADCx, ADC_CalibrationMode_Single);
  ADC_StartCalibration(ADCx);
  while(ADC_GetCalibrationStatus(ADCx) != RESET);
  ADC_Cmd(ADCx, ENABLE);
  while(!ADC_GetFlagStatus(ADCx, ADC_FLAG_RDY));
}
/* Configura los pins de captura como analogicos */
void ADC_ConfigInputs() {
  //Activo los canales ADC y los puertos involucrados
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_ADC12 | RCC_AHBPeriph_ADC34, ENABLE);
  //Configuro los 7 puertos de entrada y el de bateria
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  //Configuracion de ADCs 
  RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div1 | RCC_ADC34PLLCLK_Div1);
  ADC_InitADC(ADC1);
  ADC_InitADC(ADC2);
  ADC_InitADC(ADC3);
  ADC_InitADC(ADC4);
  //Configuracion de canales inyectados en cada conversor
  ADC_InjectedInitTypeDef ADC_InjectedInitStructure;
  ADC_InjectedInitStructure->ADC_ExternalTrigInjecConvEvent = ADC_ExternalTrigInjecConvEvent_0;    
  ADC_InjectedInitStructure->ADC_ExternalTrigInjecEventEdge = ADC_ExternalTrigInjecEventEdge_None;     
  ADC_InjectedInitStructure->ADC_NbrOfInjecChannel = 2;                                        
  ADC_InjectedInitStructure->ADC_InjecSequence1 = ADC_InjectedChannel_1; 
  ADC_InjectedInitStructure->ADC_InjecSequence2 = ADC_InjectedChannel_1;
  ADC_InjectedInit(ADC1, &ADC_InjectedInitStructure);
  ADC_InjectedInitStructure->ADC_InjecSequence1 = ADC_InjectedChannel_1; 
  ADC_InjectedInitStructure->ADC_InjecSequence2 = ADC_InjectedChannel_1;
  ADC_InjectedInit(ADC2, &ADC_InjectedInitStructure);
  ADC_InjectedInitStructure->ADC_InjecSequence1 = ADC_InjectedChannel_1; 
  ADC_InjectedInitStructure->ADC_InjecSequence2 = ADC_InjectedChannel_1;
  ADC_InjectedInit(ADC3, &ADC_InjectedInitStructure);
  ADC_InjectedInitStructure->ADC_NbrOfInjecChannel = 1;
  ADC_InjectedInitStructure->ADC_InjecSequence1 = ADC_InjectedChannel_1; 
  ADC_InjectedInit(ADC4, &ADC_InjectedInitStructure);
  
  //Configuracion de DMA

  //  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
  //NVIC_InitTypeDef        NVIC_InitStructure;  
}




void ADC_StartCapture(uint8_t mode) {
  //Configuro los buffers
  //Configuro DMAs
  //Configuro interrupciones


}