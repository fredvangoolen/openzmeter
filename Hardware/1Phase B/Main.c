#include "stm32f37x.h"
#include "USB_CDC.h"
//#include "usb_pwr.h"
#include <string.h>
//----------------------------------------------------------------------------------------------------------------------
#define INBUFFER_LEN   12800
typedef struct {
  int16_t Voltage;
  int16_t Current;
} Sample_t;
//----------------------------------------------------------------------------------------------------------------------
uint8_t  Buffer[INBUFFER_LEN];
uint16_t ReadPos   = 0;
uint16_t WritePos  = 0;
uint16_t Used      = 0;
uint8_t  LedStatus = 0x0F;
//uint32_t Sequence  = 0;
//----------------------------------------------------------------------------------------------------------------------
void SetLeds(uint8_t type) {
  if(type == 0) {
    uint8_t Val = (LedStatus >> 4) & 0x0F;
    if(Val > 8) GPIO_SetBits(GPIOB, GPIO_Pin_8);
    if(Val > 7) GPIO_SetBits(GPIOB, GPIO_Pin_7);
    if(Val > 6) GPIO_SetBits(GPIOB, GPIO_Pin_6);
    if(Val > 5) GPIO_SetBits(GPIOB, GPIO_Pin_5);
    if(Val > 4) GPIO_SetBits(GPIOB, GPIO_Pin_4);
    if(Val > 3) GPIO_SetBits(GPIOB, GPIO_Pin_3);
    if(Val > 2) GPIO_SetBits(GPIOA, GPIO_Pin_15);
    if(Val > 1) GPIO_SetBits(GPIOF, GPIO_Pin_7);
    if(Val > 0) GPIO_SetBits(GPIOF, GPIO_Pin_6);
    GPIO_SetBits(GPIOA, GPIO_Pin_0);
  } else if(type == 1) {
    uint8_t Val = LedStatus & 0x0F;
    if(Val != 0x0F) {
      if(Val > 7) GPIO_SetBits(GPIOB, GPIO_Pin_8);
      if(Val > 6) GPIO_SetBits(GPIOB, GPIO_Pin_7);
      if(Val > 5) GPIO_SetBits(GPIOB, GPIO_Pin_6);
      if(Val > 4) GPIO_SetBits(GPIOB, GPIO_Pin_5);
      GPIO_SetBits(GPIOB, GPIO_Pin_4);
      if(Val < 4) GPIO_SetBits(GPIOB, GPIO_Pin_3);
      if(Val < 3) GPIO_SetBits(GPIOA, GPIO_Pin_15);
      if(Val < 2) GPIO_SetBits(GPIOF, GPIO_Pin_7);
      if(Val < 1) GPIO_SetBits(GPIOF, GPIO_Pin_6);
    }
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
  }
}
void ClearLeds() {
  GPIO_ResetBits(GPIOA, GPIO_Pin_0);
  GPIO_ResetBits(GPIOA, GPIO_Pin_1);
  GPIO_ResetBits(GPIOB, GPIO_Pin_4); 
  GPIO_ResetBits(GPIOB, GPIO_Pin_5);
  GPIO_ResetBits(GPIOB, GPIO_Pin_6);
  GPIO_ResetBits(GPIOB, GPIO_Pin_7);
  GPIO_ResetBits(GPIOB, GPIO_Pin_8);
  GPIO_ResetBits(GPIOB, GPIO_Pin_3);
  GPIO_ResetBits(GPIOA, GPIO_Pin_15);
  GPIO_ResetBits(GPIOF, GPIO_Pin_7);
  GPIO_ResetBits(GPIOF, GPIO_Pin_6);
}
/* PC => ARM */
void EP3_OUT_Callback(void) {
  PMAToUserBufferCopy(&LedStatus, ENDP3_RXADDR, 1);
  SetEPRxValid(ENDP3);
}
/* ARM => PC */
void EP1_IN_Callback(void) {
  if(Used < VIRTUAL_COM_PORT_DATA_SIZE) return;
  UserToPMABufferCopy(&Buffer[ReadPos], ENDP1_TXADDR, VIRTUAL_COM_PORT_DATA_SIZE);
  SetEPTxCount(ENDP1, VIRTUAL_COM_PORT_DATA_SIZE);
  SetEPTxValid(ENDP1);
  ReadPos = (ReadPos + VIRTUAL_COM_PORT_DATA_SIZE) % INBUFFER_LEN;
  Used -= VIRTUAL_COM_PORT_DATA_SIZE;
//Sequence++;
}
/* Start Of Frame */
void SOF_Callback(void) {
  EP1_IN_Callback();
}
void TIM2_IRQHandler() {
  while(SDADC_GetFlagStatus(SDADC3, SDADC_FLAG_REOC) == RESET);
//  int16_t Current = SDADC_GetConversionValue(SDADC1);
 
  GPIO_ResetBits(GPIOA, GPIO_Pin_4);                                  /* SPI enable to low and setup ADC channel */
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData16(SPI1, 0xFFFF);                                   /* Load data to transfer by SPI */
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  GPIO_SetBits(GPIOA, GPIO_Pin_4);                                    /* SPI disable */
  int16_t Current = SPI_I2S_ReceiveData16(SPI1);                      /* Read current value */
  Current = (Current - 4096) * 8;
 
  int16_t Voltage = SDADC_GetConversionValue(SDADC3);
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  if(Used < INBUFFER_LEN) {
    Sample_t *Sample = (Sample_t*)&Buffer[WritePos];
    Sample->Current = Current;
    Sample->Voltage = Voltage;
    Used += sizeof(Sample_t);
    WritePos = (WritePos + sizeof(Sample_t)) % INBUFFER_LEN;
  } 
//  SDADC_ClearITPendingBit(SDADC1, SDADC_IT_JEOC | SDADC_IT_EOCAL | SDADC_IT_JOVR | SDADC_IT_ROVR);
  SDADC_ClearITPendingBit(SDADC3, SDADC_IT_JEOC | SDADC_IT_EOCAL | SDADC_IT_JOVR | SDADC_IT_ROVR);
//  SDADC_SoftwareStartConv(SDADC1);
  SDADC_SoftwareStartConv(SDADC3);
}
//Inicia los LEDs y buzzer de señalizacion al usuario
void UserSignals() {
//  GPIO_InitTypeDef        GPIO_InitStructure;
//  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOF, ENABLE);
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                       /* Output for LEDs*/
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                      /* Push-pull */
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                   /* Pin max speed 50MHz */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_4 | GPIO_Pin_3;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);                              /* Config pins */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_0 | GPIO_Pin_1;
//  GPIO_Init(GPIOA, &GPIO_InitStructure);                              /* Config pins */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6;
//  GPIO_Init(GPIOF, &GPIO_InitStructure);                              /* Config pins */

}
//Inicia el bus SPI (Solo si se lee del Infineon)
void Init_SPI() {
  SPI_InitTypeDef         SPI_InitStructure; 
  GPIO_InitTypeDef        GPIO_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);                 /* Clock used devices */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;                           /* CS pin */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                       /* Alternate function */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                      /* Push-pull */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                   /* Pin max speed 50MHz */
  GPIO_Init(GPIOA, &GPIO_InitStructure);                              /* Config pin */

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7; /* Configuring SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                        /* Alternate function */
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                        /* Pull up enabled */
  GPIO_Init(GPIOA, &GPIO_InitStructure);                              /* Config pins */

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_5);                /* Select SPI1 Alternate function*/
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_5);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_5);
  
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  /* Read only SPI */
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                       /* Master mode SPI */
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;                  /* 16 bits word size */
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                          /* Clk idle low */
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                        /* First transition is first bit */
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                           /* Software slave management */
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                  /* Byte order */
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; /* SPI prescaler */
  SPI_InitStructure.SPI_CRCPolynomial = 7;                            /* SPI CRC polynomial */
  SPI_Init(SPI1, &SPI_InitStructure);                                 /* Init SPI2 */
  SPI_CalculateCRC(SPI1, DISABLE);                                    /* Disable CRC */
  SPI_Cmd(SPI1, ENABLE);                                              /* Enable SPI2 */
}
//Inicia el ADC usado para tension
void Init_SDADC() {
  SDADC_InitTypeDef       SDADC_InitStruct;
  SDADC_AINStructTypeDef  SDADC_AINStructure;
  GPIO_InitTypeDef        GPIO_InitStructure;
  NVIC_InitTypeDef        NVIC_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOE, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  PWR_SDADCAnalogCmd(PWR_SDADCAnalog_1 | PWR_SDADCAnalog_2 | PWR_SDADCAnalog_3, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDADC1 | RCC_APB2Periph_SDADC2 | RCC_APB2Periph_SDADC3, ENABLE);
  RCC_SDADCCLKConfig(RCC_SDADCCLK_SYSCLK_Div6);                      /* Set ADC prescaler (72MHz / 12 -> 6MHz) */
  SDADC_VREFSelect(SDADC_VREF_Ext);                                  /* External reference */
  SDADC_Cmd(SDADC1, ENABLE);
  SDADC_Cmd(SDADC2, ENABLE);
  SDADC_Cmd(SDADC3, ENABLE);
  while(SDADC_GetFlagStatus(SDADC1, SDADC_FLAG_STABIP) == RESET);
  while(SDADC_GetFlagStatus(SDADC2, SDADC_FLAG_STABIP) == RESET);
  while(SDADC_GetFlagStatus(SDADC3, SDADC_FLAG_STABIP) == RESET);

  //  SDADC_InitModeCmd(SDADC1, ENABLE);                                  /* Enter initialization mode */
  SDADC_InitModeCmd(SDADC3, ENABLE);
//  while(SDADC_GetFlagStatus(SDADC1, SDADC_FLAG_INITRDY) == RESET);
  while(SDADC_GetFlagStatus(SDADC3, SDADC_FLAG_INITRDY) == RESET);

//  SDADC_InitStruct.SDADC_Channel = SDADC_Channel_6;
  SDADC_InitStruct.SDADC_ContinuousConvMode = DISABLE;
  SDADC_InitStruct.SDADC_FastConversionMode = DISABLE;
//  SDADC_Init(SDADC1, &SDADC_InitStruct);
  SDADC_InitStruct.SDADC_Channel = SDADC_Channel_8;
  SDADC_Init(SDADC3, &SDADC_InitStruct);

//  SDADC_AINStructure.SDADC_InputMode = SDADC_InputMode_Diff;/* Configure Analog input */
  SDADC_AINStructure.SDADC_Gain = SDADC_Gain_1_2;
  SDADC_AINStructure.SDADC_CommonMode = SDADC_CommonMode_VSSA;
  SDADC_AINStructure.SDADC_Offset = 0;
//  SDADC_AINInit(SDADC1, SDADC_Conf_0, &SDADC_AINStructure);
  SDADC_AINStructure.SDADC_InputMode = SDADC_InputMode_Diff;/* Configure Analog input */
  SDADC_AINInit(SDADC3, SDADC_Conf_0, &SDADC_AINStructure);
  
//  SDADC_ChannelConfig(SDADC1, SDADC_Channel_6, SDADC_Conf_0);         /* Init ADC input configuration */
  SDADC_ChannelConfig(SDADC3, SDADC_Channel_8, SDADC_Conf_0);         /* Init ADC input configuration */
  SDADC_RegularSynchroSDADC1(SDADC3, ENABLE);

//  SDADC_InitModeCmd(SDADC1, DISABLE);
  SDADC_InitModeCmd(SDADC3, DISABLE);
//  SDADC_CalibrationSequenceConfig(SDADC1, SDADC_CalibrationSequence_1);
  SDADC_CalibrationSequenceConfig(SDADC3, SDADC_CalibrationSequence_1);
//  SDADC_StartCalibration(SDADC1);
  SDADC_StartCalibration(SDADC3);
//  while(SDADC_GetFlagStatus(SDADC1, SDADC_FLAG_EOCAL) == RESET);
  while(SDADC_GetFlagStatus(SDADC3, SDADC_FLAG_EOCAL) == RESET);
//  SDADC_SoftwareStartConv(SDADC1);
  SDADC_SoftwareStartConv(SDADC3);
}
void Init_Timer() {
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef        NVIC_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);                /* Set clock source for TIM2 */
  TIM_TimeBaseStructure.TIM_Prescaler = 0;                            /* Timer prescaler -> 1uS */
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;         /* Count mode */
  TIM_TimeBaseStructure.TIM_Period = 4607;                            /* Reset period (16kHz) */
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;             /* */
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;                    /* */
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);                     /* Configure TIM2 */
  TIM_Cmd(TIM2, ENABLE);                                              /* Enable TIM2 */ 
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);                          /* Enable overflow interrupt */

//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;                     /* Interruption source */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;           /* Priority */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                  /* Subpriority */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                     /* Enable */
  NVIC_Init(&NVIC_InitStructure);                                     /* Configure TIM2 interrupt */
}
void Init_USB_HW() {
  NVIC_InitTypeDef        NVIC_InitStructure;         
  GPIO_InitTypeDef        GPIO_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_IRQn;                   /* Interruption source */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;           /* Priority */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                  /* Subpriority */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                     /* Enable */
  NVIC_Init(&NVIC_InitStructure);                                     /* Configure USB interrupt */
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;            /* Configure USB pins */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                   /* High speed */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                        /* Alternate function */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                      /* Push-Pull */
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;                    /* */
  GPIO_Init(GPIOA, &GPIO_InitStructure);                              /* Set configuration */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_14);              /* Usb pins alternate function */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_14);
  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);                    /* Select USBCLK source */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);                 /* Enable the USB clock */
	USB_Init();
}
int main(void) {
  //Init_SPI();
  Init_SDADC();
  Init_SPI();
    Init_Timer();
  Init_USB_HW();


  
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);                /* Set clock source for TIM2 */
//  TIM_TimeBaseStructure.TIM_Prescaler = 71;                           /* Timer prescaler -> 1uS */
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;         /* Count mode */
//  TIM_TimeBaseStructure.TIM_Period = 63;                              /* Reset period (64uS) */
//  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;             /* */
//  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;                    /* */
//  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);                     /* Configure TIM2 */
//  TIM_Cmd(TIM2, ENABLE);                                              /* Enable TIM2 */ 
//  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);                          /* Enable overflow interrupt */

//  
//  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;                     /* Interruption source */
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;           /* Priority */
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                  /* Subpriority */
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                     /* Enable */
//  NVIC_Init(&NVIC_InitStructure);                                     /* Configure TIM2 interrupt */

  while(1) {

  }
}