/* ================================ openZmeter =========================================================================
 * File:     ADC_Capture.h
 * Author:   Eduardo Viciana (2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#include "Main.h"
#include <arm_math.h>
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
  int16_t CurrentN;  //ADC1_IN4
  int16_t CurrentR;  //ADC2_IN12
  int16_t Battery;   //ADC1_IN1
  int16_t VoltageR;  //ADC2_IN1
} ADCSample_ADC12;
typedef struct {
  int16_t CurrentS;  //ADC3_IN5
  int16_t CurrentT;  //ADC4_IN5
  int16_t VoltageS;  //ADC3_IN12
  int16_t VoltageT;  //ADC4_IN3
} ADCSample_ADC34;
//Variables locales ----------------------------------------------------------------------------------------------------
static Filter_t        Filters;          //Almacena los coeficientes y muestras de los filtros de cada canal
static uint8_t         ChannelMask;      //Mascara de canales en uso
static uint8_t         SampleCounter;    //La parte alta tiene el tipo de oversample y la baja el contador de muestras procesadas
static uint8_t         Channels;         //Numero de canales activos
static ADCSample_ADC12 ADC12_Buffer[32]; //Buffer de captura para DMA1
static ADCSample_ADC34 ADC34_Buffer[32]; //Buffer de captura para DMA2
//Tipos y funciones externas -------------------------------------------------------------------------------------------
extern void AppendSample(int16_t value);
//----------------------------------------------------------------------------------------------------------------------

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
  //Configuro ADC
  ADC_VoltageRegulatorCmd(ADC1, ENABLE);
  ADC_VoltageRegulatorCmd(ADC2, ENABLE);
  ADC_VoltageRegulatorCmd(ADC3, ENABLE);
  ADC_VoltageRegulatorCmd(ADC4, ENABLE);
  Waitms(1);
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_RegSimul;
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_Circular;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;
  ADC_CommonInit(ADC1, &ADC_CommonInitStructure);
  ADC_CommonInit(ADC2, &ADC_CommonInitStructure);
  ADC_CommonInit(ADC3, &ADC_CommonInitStructure);
  ADC_CommonInit(ADC4, &ADC_CommonInitStructure);
  ADC_InitTypeDef ADC_InitStructure;
  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_2;
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_RisingEdge;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;
  ADC_InitStructure.ADC_NbrOfRegChannel = 2;
  ADC_Init(ADC1, &ADC_InitStructure);
  ADC_Init(ADC2, &ADC_InitStructure);
  ADC_Init(ADC3, &ADC_InitStructure);
  ADC_Init(ADC4, &ADC_InitStructure);
  ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);
  ADC_SelectCalibrationMode(ADC2, ADC_CalibrationMode_Single);
  ADC_SelectCalibrationMode(ADC3, ADC_CalibrationMode_Single);
  ADC_SelectCalibrationMode(ADC4, ADC_CalibrationMode_Single);  
  ADC_StartCalibration(ADC1);
  ADC_StartCalibration(ADC2);
  ADC_StartCalibration(ADC3);
  ADC_StartCalibration(ADC4);
  while((ADC_GetCalibrationStatus(ADC1) != RESET) || (ADC_GetCalibrationStatus(ADC2) != RESET) || 
        (ADC_GetCalibrationStatus(ADC3) != RESET) || (ADC_GetCalibrationStatus(ADC4) != RESET));
  ADC_Cmd(ADC1, ENABLE);
  ADC_Cmd(ADC2, ENABLE);
  ADC_Cmd(ADC3, ENABLE);
  ADC_Cmd(ADC4, ENABLE);
  while((ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY) == RESET) || (ADC_GetFlagStatus(ADC2, ADC_FLAG_RDY) == RESET) || 
        (ADC_GetFlagStatus(ADC3, ADC_FLAG_RDY) == RESET) || (ADC_GetFlagStatus(ADC4, ADC_FLAG_RDY) == RESET));
  ADC_RegularChannelConfig(ADC1, ADC_InjectedChannel_4,  1, ADC_SampleTime_1Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_InjectedChannel_1,  2, ADC_SampleTime_1Cycles5);
  ADC_RegularChannelConfig(ADC2, ADC_InjectedChannel_12, 1, ADC_SampleTime_1Cycles5);
  ADC_RegularChannelConfig(ADC2, ADC_InjectedChannel_1,  2, ADC_SampleTime_1Cycles5);
  ADC_RegularChannelConfig(ADC3, ADC_InjectedChannel_5,  1, ADC_SampleTime_1Cycles5);
  ADC_RegularChannelConfig(ADC3, ADC_InjectedChannel_12, 2, ADC_SampleTime_1Cycles5);
  ADC_RegularChannelConfig(ADC4, ADC_InjectedChannel_5,  1, ADC_SampleTime_1Cycles5);
  ADC_RegularChannelConfig(ADC4, ADC_InjectedChannel_3,  2, ADC_SampleTime_1Cycles5);
  ADC_DMAConfig(ADC1, ADC_DMAMode_Circular);
  ADC_DMACmd(ADC1, ENABLE);
  ADC_DMAConfig(ADC3, ADC_DMAMode_Circular);
  ADC_DMACmd(ADC3, ENABLE);
  ADC_StartConversion(ADC1);
  ADC_StartConversion(ADC2);
  ADC_StartConversion(ADC3);
  ADC_StartConversion(ADC4);
  //Inicio DMA
  DMA_InitTypeDef DMA_InitStructure;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC12_Buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 64;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  DMA_Cmd(DMA1_Channel1, ENABLE);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC3->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC34_Buffer;
  DMA_Init(DMA2_Channel5, &DMA_InitStructure);
  DMA_Cmd(DMA2_Channel5, ENABLE);
  //Habilito interrupciones del DMA solo uso un canal, ya que todas las operaciones finalizan a la vez
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC | DMA_IT_HT, ENABLE);
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
 * Inicia la captura de muestras, iniciando los parametros de los filtros si son necesarios
 * @param channels mascara de bits que indica que canales capturar el orden es [xx Vr Cr Vs Cs Vt Ct Cn]
 * @param mode modo de captura, los dos bits mas significativos son el oversample (00=1x, 01=4x, 10=16x, 11=64x)
 *             los demas bits determinan la velocidad de captura
 * @return el valor devuelto es true si los parametros son validos y la captura se inicia correctamente
 */
const uint16_t ADC_TimerDivs[] = { 150,  180,  200,  225,  250,  300,  260,  375,  450,  500,  600,  625,  750,  900, 1000,  1125,  1250, 
                                  1500, 1800, 1875, 2250, 2500, 3000, 3125, 3750, 4500, 5000, 5625, 6250, 7500, 9000, 9375, 11250, 12500};
bool ADC_StartCapture(uint8_t channels, uint8_t mode) {
  //Recuento de canales activos
  Channels = 0;
  for(uint8_t Mask = 0x40; Mask != 0; Mask >>= 1) {
    if(channels & Mask) Channels++;
  }
  if((Channels == 0) || ((mode & 0x3F) > 33)) return FALSE;
  uint32_t Freq = SystemCoreClock / ADC_TimerDivs[mode & 0x3F];
  if((mode & 0xC0) == 0x40) Freq = Freq / 4;
  if((mode & 0xC0) == 0x80) Freq = Freq / 16;
  if((mode & 0xC0) == 0xC0) Freq = Freq / 64;
  if(((Freq * Channels) > 360000) || (Freq < 5000)) return FALSE;
  ADC_StopCapture();
  ChannelMask = channels;
  SampleCounter = mode & 0xC0;
  for(int i = 0; i < Channels; i++) {
    Filters.Z1[i] = 0.0;
    Filters.Z2[i] = 0.0;
  }
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
  Buffers_Init(Channels);
  //Configuro TIMER1 CC3 que es compartido por los 4 ADCs
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseStructure.TIM_Period = ADC_TimerDivs[mode & 0x3F] - 1;
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

/**
 * Filtra una muestra en de entrada segun los parametros configurados del filtro
 * @param in muestra de entrada a añadir al filtro
 * @param channel canal de la muestra
 * @return filtered value
 */
static inline float ADC_Filter(float in, uint8_t channel) {
  if(ChannelMask & 0xC0) {
    float Out = in * Filters.A0 + Filters.Z1[channel];
    Filters.Z1[channel] = in * Filters.A1 + Filters.Z2[channel] - Filters.B1 * Out;
    Filters.Z2[channel] = in * Filters.A2 - Filters.B2 * Out;
    return Out;
  } else {
    return in;
  }
}

/**
 * Control de la interrupcion del DMA del ADC, se llama cada vez que tenemos 16
 * muestras para procesar (medio buffer). Los pasos en cada interrupcion son
 * determinar el trozo de buffer a procesar, pasar por el filtro los canales
 * activos, 
 * 
 */
void DMA1_Channel1_IRQHandler(void) {
  float   Values[7];
  uint8_t Start;
  if(DMA_GetITStatus(DMA1_IT_TC1) != RESET) {
    Start = 16;
    DMA_ClearITPendingBit(DMA1_IT_TC1); 
  } else {
    Start = 0;
    DMA_ClearITPendingBit(DMA1_IT_HT1);
  }
  
  DMA_ClearITPendingBit(DMA1_IT_HT1 | DMA1_IT_TC1);
  for(uint8_t Pos = Start; Pos < Start + 16; Pos++) {
    float *Ptr = Values;
    if(ChannelMask & 0x40) *Ptr++ = ADC12_Buffer[Pos].VoltageR;
    if(ChannelMask & 0x20) *Ptr++ = ADC12_Buffer[Pos].CurrentR;
    if(ChannelMask & 0x10) *Ptr++ = ADC34_Buffer[Pos].VoltageS;
    if(ChannelMask & 0x08) *Ptr++ = ADC34_Buffer[Pos].CurrentS; 
    if(ChannelMask & 0x04) *Ptr++ = ADC34_Buffer[Pos].VoltageT;
    if(ChannelMask & 0x02) *Ptr++ = ADC34_Buffer[Pos].CurrentT;
    if(ChannelMask & 0x01) *Ptr++ = ADC12_Buffer[Pos].CurrentN;
   
    if(SampleCounter == 0x00) {
      Buffer_AppendSamples(Values);
    } else {
      float *Z1 = Filters.Z1;
      float *Z2 = Filters.Z2;
      Ptr = Values;
      for(uint8_t Channel = 0; Channel < 7; Channel++) {
        float Out = *Ptr * Filters.A0 + *Z1;
        *Z1 = *Ptr * Filters.A1 + *Z2 - Filters.B1 * Out;
        *Z2 = *Ptr * Filters.A2 - Filters.B2 * Out;
        *Ptr = Out;
        Ptr++;
        Z1++;
        Z2++;
        if((SampleCounter == 0x43) || (SampleCounter == 0x8F) || (SampleCounter == 0xFF)) {
          Buffer_AppendSamples(Values);
          SampleCounter = SampleCounter & 0xC0;
        } else {
          SampleCounter++;
        }  
      }
    }
  }
  
/*
  for(uint8_t Count = 0; Count < 16; Count++) {
    float *Ptr = Values;
    if(ChannelMask & 0x40) *Ptr++ = ADC_Filter(ADC2_Buffer[Start + Count].VoltageR, 6);
    if(ChannelMask & 0x20) *Ptr++ = ADC_Filter(ADC2_Buffer[Start + Count].CurrentR, 5);
    if(ChannelMask & 0x10) *Ptr++ = ADC_Filter(ADC3_Buffer[Start + Count].VoltageS, 4);
    if(ChannelMask & 0x08) *Ptr++ = ADC_Filter(ADC3_Buffer[Start + Count].CurrentS, 3);
    if(ChannelMask & 0x04) *Ptr++ = ADC_Filter(ADC4_Buffer[Start + Count].VoltageT, 2);
    if(ChannelMask & 0x02) *Ptr++ = ADC_Filter(ADC4_Buffer[Start + Count].CurrentT, 1);
    if(ChannelMask & 0x01) *Ptr++ = ADC_Filter(ADC1_Buffer[Start + Count].CurrentN, 0);
  }
*/
}