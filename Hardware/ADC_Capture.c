/* ================================ openZmeter =========================================================================
 * File:     ADC_Capture.h
 * Author:   Eduardo Viciana (2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#include <arm_math.h>
//#include <stdbool.h>
#include "Main.h"
//Definiciones locales -------------------------------------------------------------------------------------------------
typedef struct {
  float A0;
  float A1;
  float A2;
  float B1;
  float B2;
  float Z1[7];
  float Z2[7];
} Filter_t;
typedef struct {
  int16_t CurrentR;  //ADC1_IN4
  int16_t Battery;   //ADC1_IN1
} ADCSample_ADC1;
typedef struct {
  int16_t CurrentS;  //ADC2_IN12
  int16_t VoltageR;  //ADC2_IN1
} ADCSample_ADC2;
typedef struct {
  int16_t CurrentN;  //ADC3_IN5
  int16_t VoltageS;  //ADC3_IN12
} ADCSample_ADC3;
typedef struct {
  int16_t CurrentT;  //ADC4_IN5
  int16_t VoltageT;  //ADC4_IN3
} ADCSample_ADC4;
//Variables locales ----------------------------------------------------------------------------------------------------
static Filter_t       Filters;         //Almacena los coeficientes y muestras de los filtros de cada canal
static uint8_t        ChannelMask;     //Mascara de canales en uso
static uint8_t        SampleCounter;   //La parte alta tiene el tipo de oversample y la baja el contador de muestras procesadas
static ADCSample_ADC1 ADC1_Buffer[32]; //Buffer de captura para Voltage_R y Current_R
static ADCSample_ADC2 ADC2_Buffer[32]; //Buffer de captura para Voltage_S y Current_S
static ADCSample_ADC3 ADC3_Buffer[32]; //Buffer de captura para Voltage_T y Current_T
static ADCSample_ADC4 ADC4_Buffer[32]; //Buffer de captura para Battery y Current_N
//Tipos y funciones externas -------------------------------------------------------------------------------------------
extern void AppendSample(int16_t value);
//----------------------------------------------------------------------------------------------------------------------
/**
 * Configura la captura de un ADC para leer dos canales secuencialmente.
 * @param ADCx ADC a configurar ADC1, ADC2, ADC3 o ADC4
 * @param channel1 primer canal de entrada al ADC
 * @param channel2 segundo canal de entrada al ADC
 * @param dst direccion del buffer del DMA usado para guardar las muestras
 * @param dma controador y canal DMA usado para la transferencia
 */
void ADC_InitADC(ADC_TypeDef* ADCx, uint8_t channel1, uint8_t channel2, uint16_t *dst, DMA_Channel_TypeDef *dma) {
  //Configuro ADC
  ADC_InitTypeDef ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_VoltageRegulatorCmd(ADCx, ENABLE);
  for(int i = 0; i < 10000; i++);
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_Circular;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;
  ADC_CommonInit(ADCx, &ADC_CommonInitStructure);
  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_2;
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_RisingEdge;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
  ADC_InitStructure.ADC_NbrOfRegChannel = 2;
  ADC_Init(ADCx, &ADC_InitStructure);
  ADC_SelectCalibrationMode(ADCx, ADC_CalibrationMode_Single);
  ADC_StartCalibration(ADCx);
  while(ADC_GetCalibrationStatus(ADCx) != RESET);
  ADC_Cmd(ADCx, ENABLE);
  while(!ADC_GetFlagStatus(ADCx, ADC_FLAG_RDY));
  ADC_DMAConfig(ADCx, ADC_DMAMode_Circular);
  //Configuro secuencia de captura
  ADC_RegularChannelConfig(ADCx, channel1,  1, ADC_SampleTime_1Cycles5);
  ADC_RegularChannelConfig(ADCx, channel2,  2, ADC_SampleTime_1Cycles5);
  ADC_DMACmd(ADCx, ENABLE);
  //Configuro DMA
  DMA_InitTypeDef  DMA_InitStructure;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADCx->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dst;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 64;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(dma, &DMA_InitStructure);
  DMA_Cmd(dma, ENABLE);
  ADC_StartConversion(ADCx);
}
/**
 * Configura todos los ADCs usados, timer necesario para la captura, controlador DMA e interrupciones
 */
void ADC_Configure() {
  //Activo los canales ADC y los puertos involucrados
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_ADC12 | RCC_AHBPeriph_ADC34 | RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_DMA2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
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
  ADC_InitADC(ADC4, ADC_InjectedChannel_5,  ADC_InjectedChannel_3,  (uint16_t*)ADC4_Buffer, DMA2_Channel2);
  ADC_InitADC(ADC3, ADC_InjectedChannel_5,  ADC_InjectedChannel_12, (uint16_t*)ADC3_Buffer, DMA2_Channel5);
  ADC_InitADC(ADC2, ADC_InjectedChannel_12, ADC_InjectedChannel_1,  (uint16_t*)ADC2_Buffer, DMA2_Channel1);
  ADC_InitADC(ADC1, ADC_InjectedChannel_4,  ADC_InjectedChannel_1,  (uint16_t*)ADC1_Buffer, DMA1_Channel1);
  //Habilito interrupciones del DMA solo uso un canal, ya que todas las operaciones finalizan mas o menos a la vez
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC | DMA_IT_HT, ENABLE);
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
float ADC_Filter(float in, Filter_t* filter) {
//  float out = in * filter->A0 + filter->Z1;
//  filter->Z1 = in * filter->A1 + filter->Z2 - filter->B1 * out;
//  filter->Z2 = in * filter->A2 - filter->B2 * out;
//  return out;
  return 0.0;
}
/**
 * Inicia la captura de muestras, iniciando los parametros de los filtros si son necesarios
 * @param channels mascara de bits que indica que canales capturar el orden es [xx Vr Cr Vs Cs Vt Ct Cn]
 * @param mode modo de captura, los dos bits mas significativos son el oversample (00=1x, 01=4x, 10=16x, 11=64x)
 *             los demas bits determinan la velocidad de captura
 * @return el valor devuelto es true si los parametros son validos y la captura se inicia correctamente
 */
bool ADC_StartCapture(uint8_t channels, uint8_t mode) {
  ADC_StopCapture();
  ChannelMask = channels;
  SampleCounter = mode & 0xC0;
  for(int i = 0; i < 7; i++) {
    Filters.Z1[i] = 0.0;
    Filters.Z2[i] = 0.0;
  }
  //Recuento de canales activos
  uint8_t Channels = 0;
  for(uint8_t Mask = 0x80; Mask != 0; Mask >>= 1) {
    if(channels & Mask) Channels++;
  }
  if(Channels == 0) return;
  
//Busco las frequencias y canales que hay para determinar si puedo procesar o es excesivo

  float Q = 1.0f / sqrt(2.0f);
  float Fc = 1.0f;
  if(SampleCounter == 0xC0) Fc = Fc / 4.0;
  if(SampleCounter == 0xC0) Fc = Fc / 16.0;
  if(SampleCounter == 0xC0) Fc = Fc / 64.0;
  float K    = tan(PI * Fc);
  float Norm = 1 / (1 + K / Q + K * K);
  Filters.A0 = K * K * Norm;
  Filters.A1 = 2 * Filters.A0;
  Filters.A2 = Filters.A0;
  Filters.B1 = 2 * (K * K - 1) * Norm;
  Filters.B2 = (1 - K / Q + K * K) * Norm;
//  ResetBuffers();
  //Configuro TIMER1 CC3 que es compartido por los 4 ADCs
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseStructure.TIM_Period = 89;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
  TIM_OCInitTypeDef TIM_OCInitStructure;
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 1;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OC3Init(TIM1, &TIM_OCInitStructure);
  TIM_CtrlPWMOutputs(TIM1, ENABLE);
  TIM_Cmd(TIM1, ENABLE);
  return TRUE;
}
/**
 * Detiene la captura
 */
void ADC_StopCapture() {
  TIM_Cmd(TIM1, DISABLE);
}
void AppendBuffer(uint16_t sample) {
  INBuff[INWritePos] = sample;
  INBuffUsed++;
  INWritePos = (INWritePos + 1) % INBUFFER_LEN;
}

void DMA1_Channel1_IRQHandler(void) {
  uint8_t  Start;
//  uint16_t Accum = 0;
  if(DMA_GetITStatus(DMA1_IT_TC1)) {
    Start = 0;
    DMA_ClearITPendingBit(DMA1_IT_TC1);
  } else if(DMA_GetITStatus(DMA1_IT_HT1)) {
    Start = 16;
    DMA_ClearITPendingBit(DMA1_IT_HT1);
  }
  __disable_irq();
//  if(INBuffUsed >= (INBUFFER_LEN - ChannelsCount)) {
//    ResetBuffers();
//  } else {
//    if((ChannelMask & 0x01) == 0x01) AppendBuffer(ADC2_Buffer[Start].VoltageR);
//    if((ChannelMask & 0x02) == 0x02) AppendBuffer(ADC3_Buffer[Start].VoltageS);
//    if((ChannelMask & 0x04) == 0x04) AppendBuffer(ADC4_Buffer[Start].VoltageT);
//    if((ChannelMask & 0x08) == 0x08) AppendBuffer(ADC1_Buffer[Start].CurrentR);
//    if((ChannelMask & 0x10) == 0x10) AppendBuffer(ADC2_Buffer[Start].CurrentS);
//    if((ChannelMask & 0x20) == 0x20) AppendBuffer(ADC4_Buffer[Start].CurrentT);
//    if((ChannelMask & 0x40) == 0x40) AppendBuffer(ADC3_Buffer[Start].CurrentN);
//  }
  __enable_irq();
}