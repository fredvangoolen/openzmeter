#include "stm32f10x.h"
#include "usb_lib.h"
#include "USB.h"
#include "usb_pwr.h"
#include <string.h>
//Tipos de datos -------------------------------------------------------------------------------------------------------
#define INBUFFER_LEN   1024
typedef struct {
  int16_t Voltage[3];
  int16_t Current[4];
} Sample_t;
//----------------------------------------------------------------------------------------------------------------------
Sample_t INBuff[INBUFFER_LEN];
uint64_t Samples = 0;
uint16_t INRead  = 0;
uint16_t INWrite = 0;
uint16_t INSize  = 0;
//----------------------------------------------------------------------------------------------------------------------
/*void UsbSendData(const uint8_t *data, int len) {
  for (int i = 0; i < len; i++) {
    USART_Rx_Buffer[USART_Rx_ptr_in++] = data[i];
    if(USART_Rx_ptr_in == USART_RX_DATA_SIZE) {
      USART_Rx_ptr_in = 0;
      USART_Rx_Buffer[USART_Rx_ptr_in++] = 0xFF; 
      USART_Rx_Buffer[USART_Rx_ptr_in++] = 0xFF;
      USART_Rx_Buffer[USART_Rx_ptr_in++] = 0xFF; 
      USART_Rx_Buffer[USART_Rx_ptr_in++] = 0xFF;
    }
  }
}*/


/* ARM => PC */
void EP1_IN_Callback(void) {

		/* Endpoint type is Bulk and Double Buffer enabled*/
		
		if (GetENDPOINT(ENDP1 & 0x7F) & EP_DTOG_RX) { // NOT TX ie SW_BUF
//			UserToPMABufferCopy(pBufferPointer, GetEPDblBuf0Addr(ENDP1 & 0x7F), wBufferSize);
			SetEPDblBuf0Count((ENDP1 & 0x7F), EP_DBUF_IN, 64);
		} else {
//			UserToPMABufferCopy(pBufferPointer, GetEPDblBuf1Addr(bEpAddr & 0x7F), wBufferSize);
			SetEPDblBuf1Count((ENDP1 & 0x7F), EP_DBUF_IN, 64);
		}
		FreeUserBuffer((ENDP1 & 0x7F), EP_DBUF_IN); // Toggles EP_DTOG_RX / SW_BUF

  
  
  
  
  
  
  
 // if(INSize < 64) return;
  //UserToPMABufferCopy(&INBuff[INRead], ENDP1_TXADDR, 64);
//  SetEPTxCount(ENDP1, 64);
//  SetEPTxValid(ENDP1);
 // INRead = (INRead + 64) % INBUFFER_LEN;
 // INSize -= 64;
}
/* Start Of Frame */
void SOF_Callback(void) {
  EP1_IN_Callback();
}
void TIM2_IRQHandler() {
  if(TIM_GetITStatus(TIM2, TIM_IT_Update) == RESET) return;
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//  if((fPortOpen == false) || (INSize >= INBUFFER_LEN)) {
//    GPIO_SetBits(GPIOC, GPIO_Pin_13);                                 /* Turn OFF connection/congestion LED */
//    return;
//  }
  GPIO_ResetBits(GPIOC, GPIO_Pin_13);                                 /* Turn ON connection/congestion LED */
  Sample_t *Sample = (Sample_t*)&INBuff[INWrite];
  GPIO_ResetBits(GPIOB, GPIO_Pin_12);                                 /* SPI enable to low and setup ADC channel */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_13Cycles5);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI2, 0xFFFF);                                     /* Load data to transfer by SPI */
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);                             /* Start ADC conversion */
  while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);              /* Wait for completion of reads */
  GPIO_SetBits(GPIOB, GPIO_Pin_12);                                   /* SPI disable */
//  Sample->Voltage = ADC_GetConversionValue(ADC1) & 0x0FFF;            /* Read voltage value */
//  Sample->Current = SPI_I2S_ReceiveData(SPI2)  & 0x0FFF;              /* Read current value */
  INWrite = (INWrite + sizeof(Sample_t)) % INBUFFER_LEN;
  INSize += sizeof(Sample_t);
}

int main(void) {
  ADC_InitTypeDef         ADC_InitStructure;
  SPI_InitTypeDef         SPI_InitStructure; 
  GPIO_InitTypeDef        GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
  NVIC_InitTypeDef        NVIC_InitStructure;         
  RCC_ADCCLKConfig(RCC_PCLK2_Div6);                                   /* Set ADC prescaler (72MHz / 6 -> 12MHz) */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);                /* Select ADC1 clock source */
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  /* ADC1 and ADC2 works independenly */
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;                       /* Disable the scan conversion so we do one at a time */
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                 /* Don't do contimuous conversions - do them on demand */
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; /* Start conversin by software, not an external trigger */
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;              /* Conversions are 12 bit - put them in the lower 12 bits of the result */
  ADC_InitStructure.ADC_NbrOfChannel = 1;                             /* Say how many channels would be used by the sequencer */
  ADC_Init(ADC1, &ADC_InitStructure);                                 /* Init ADC1 */
  ADC_Cmd(ADC1, ENABLE);                                              /* Enable ADC1 */
  ADC_ResetCalibration(ADC1);                                         /* Enable ADC1 reset calibaration register */
  while(ADC_GetResetCalibrationStatus(ADC1));                         /* Check the end of ADC1 reset calibration register */
  ADC_StartCalibration(ADC1);                                         /* Start ADC1 calibaration */
  while(ADC_GetCalibrationStatus(ADC1));                              /* Check the end of ADC1 calibration */
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;                           /* Config ANALOG IN on pin 0*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                       /* Setup mode */
  GPIO_Init(GPIOA, &GPIO_InitStructure);                              /* Config pin */
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);                /* Set clock source for SPI devices */

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;                          /* Configuring congestion LED */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;                    /* Alternate function */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                   /* Pin max speed 50MHz */
  GPIO_Init(GPIOC, &GPIO_InitStructure);                              /* Config pins */

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;            /* Configuring SCK, MOSI */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;                     /* Alternate function */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                   /* Pin max speed 50MHz */
  GPIO_Init(GPIOB, &GPIO_InitStructure);                              /* Config pins */

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;                          /* Configuring MISO */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                   /* Pin max speed 50MHz */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;                       /* Input with pullup */
  GPIO_Init(GPIOB, &GPIO_InitStructure);                              /* Config pins */
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;                          /* CS pin */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;                    /* Output push pull */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                   /* Pin max speed 50MHz */
  GPIO_Init(GPIOB, &GPIO_InitStructure);                              /* Config pin */

  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  /* Read only SPI */
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                       /* Master mode SPI */
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;                  /* 16 bits word size */
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                          /* Clk idle low */
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                        /* First transition is first bit */
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                           /* Software slave management */
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                  /* Byte order */
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; /* SPI prescaler */
  SPI_InitStructure.SPI_CRCPolynomial = 7;                            /* SPI CRC polynomial */
  SPI_Init(SPI2, &SPI_InitStructure);                                 /* Init SPI2 */
  SPI_CalculateCRC(SPI2, DISABLE);                                    /* Disable CRC */
  SPI_Cmd(SPI2, ENABLE);                                              /* Enable SPI2 */

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);                /* Set clock source for TIM2 */
  TIM_TimeBaseStructure.TIM_Prescaler = 71;                           /* Timer prescaler -> 1uS */
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;         /* Count mode */
  TIM_TimeBaseStructure.TIM_Period = 24;                              /* Reset period (25uS) */
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;             /* */
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;                    /* */
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);                     /* Configure TIM2 */
  TIM_Cmd(TIM2, ENABLE);                                              /* Enable TIM2 */ 
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);                          /* Enable overflow interrupt */

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;                     /* Interruption source */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;           /* Priority */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                  /* Subpriority */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                     /* Enable */
  NVIC_Init(&NVIC_InitStructure);                                     /* Configure TIM2 interrupt */

  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;          /* Interruption source */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;           /* Priority */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                  /* Subpriority */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                     /* Enable */
  NVIC_Init(&NVIC_InitStructure);                                     /* Configure USB interrupt */
  

  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);                    /* Select USBCLK source */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);                 /* Enable the USB clock */
	USB_Init();

	while(1) {

  
  
  
  
  
  
  }
}