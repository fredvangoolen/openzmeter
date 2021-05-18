#include <stdbool.h>
#include <stdint.h>
#include <STM32F0xx.h>
#include <string.h>
#include "USB.h"
// Configuration -------------------------------------------------------------------------------------------------------
#define BUFF_LEN                        2816
// USB application definitions -----------------------------------------------------------------------------------------
#define SEND_EP                         0x81
// Local variables -----------------------------------------------------------------------------------------------------
uint16_t          ADC_Read[8][9];       //DMA Buffer
uint16_t          Buff_Head;            //Write position
uint16_t          Buff_Tail;            //Read position
uint16_t          Buff_Data[BUFF_LEN];  //Buffer storage
uint8_t           LED_Counter;          //LED blink counter
// Control LED, buton and buzzer ---------------------------------------------------------------------------------------
void SysTick_ISR() {
  if(LED_Counter == 0) {
    GPIOF->BSRR = GPIO_BSRR_BR_0;
  } else {
    LED_Counter--;
    GPIOF->ODR ^= GPIO_ODR_0;
  }
}
// DMA interrupts to handle ADC oversample -----------------------------------------------------------------------------
void DMA_CH1_ISR() {
  uint16_t *Ptr;
  if(DMA1->ISR & DMA_ISR_HTIF1) {
    Ptr = ADC_Read[0];
    DMA1->IFCR |= DMA_IFCR_CHTIF1;
  } else {
    Ptr = ADC_Read[4];
    DMA1->IFCR |= DMA_IFCR_CTCIF1;
  }
  for(uint8_t Sample = 0; Sample < 9; Sample++) {
    uint16_t Next = Buff_Head + 1;
    if(Next == BUFF_LEN) Next = 0;
    if(Next != Buff_Tail) {
      uint16_t Tmp = ((Ptr[0] + Ptr[9] + Ptr[18] + Ptr[27]) >> 1) & 0x1FFF;
      Tmp |= ((Sample == 0) ? 0x8000 : 0xC000);
      Buff_Data[Buff_Head] = Tmp;
      Buff_Head = Next;
    } else {
      Buff_Head  = 0;
      Buff_Tail  = 0;
    }
    Ptr++;
  }
}
// USB callbacks functions ---------------------------------------------------------------------------------------------
void USB_HandleEP1_TX(uint8_t event) {
  if(event != USB_EVENT_TX) return;
  PMA_Table_t  *Tbl   = EPT(SEND_EP);
  uint16_t     *Reg   = EPR(SEND_EP);
  PMA_Record_t *Ptr   = (*Reg & USB_EP_SWBUF_TX) ? &(Tbl->tx1) : &(Tbl->tx0);
  uint16_t     *Pma   = (void*)(USB_PMAADDR + Ptr->addr);
  for(int x = 0; x < 32; x++) {
    uint16_t Word;
    uint8_t *Bytes = (uint8_t*)&Word;
    if(Buff_Head == Buff_Tail) {
      Word = 0x0001;
    } else {
      Word = Buff_Data[Buff_Tail];
      Buff_Tail = Buff_Tail + 1;
      if(Buff_Tail == BUFF_LEN) Buff_Tail = 0;
    }
    *Pma++ = Bytes[1] << 8 | Bytes[0];
  }
  LED_Counter = 16;
  Ptr->cnt    = 64;
  *Reg = (*Reg & USB_EPREG_MASK) | USB_EP_SWBUF_TX;
}
USB_Response USB_SetConfiguration(uint8_t cfg) {
  switch(cfg) {
    case 0:
      GPIOF->BSRR = GPIO_BSRR_BR_1;
      USB_DeconfigEP(SEND_EP);
      USB_RegisterEP(SEND_EP, 0);
      return USB_ACK;
    case 1:
      GPIOF->BSRR = GPIO_BSRR_BS_1;
      USB_ConfigEP(SEND_EP, USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF, 64);
      USB_RegisterEP(SEND_EP, USB_HandleEP1_TX);
      return USB_ACK;
    default:
      return USB_FAIL;
  }
}
// Prepare device to run after wakeup or power up ----------------------------------------------------------------------
void Setup() {
  //Setup FLASH access time
  FLASH->ACR |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
  //Clock initialization and power devices
  __enable_irq();
  RCC->CR2 |= RCC_CR2_HSI48ON | RCC_CR2_HSI14ON;
  while(((RCC->CR2 & RCC_CR2_HSI48RDY) == 0) || ((RCC->CR2 & RCC_CR2_HSI14RDY) == 0));
  RCC->CFGR |= RCC_CFGR_SW_HSI48;
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOFEN | RCC_AHBENR_DMA1EN;
  RCC->APB1ENR |= RCC_APB1ENR_USBEN | RCC_APB1ENR_CRSEN | RCC_APB1ENR_PWREN | RCC_APB1ENR_TIM3EN;
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN | RCC_APB2ENR_ADC1EN;
  CRS->CR |= CRS_CR_AUTOTRIMEN | CRS_CR_CEN;
  //Configure port F (PF0, PF1 -> Digital outputs)
  GPIOF->MODER   |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0;
  GPIOF->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR1;
  //Configure Port A (PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7 -> Analog input, PA11, PA12 -> Alternate function, PA13, PA14 -> SWD)
  GPIOA->MODER |= GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7 |
                  GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1 ;
  SYSCFG->CFGR1 |= SYSCFG_CFGR1_PA11_PA12_RMP;
  //Configure Port B (PB1 -> Analog inputs)
  GPIOB->MODER |= GPIO_MODER_MODER1;
  //Enable DMA controller
  DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
  DMA1_Channel1->CMAR = (uint32_t)ADC_Read;
  DMA1_Channel1->CNDTR = 72;
  DMA1_Channel1->CCR |= DMA_CCR_PL_0 | DMA_CCR_PL_1 | DMA_CCR_PSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_MSIZE_0 | DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_EN;
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
  // Initialize the ADC (14MHz, 1.5 adquisition, DMA active, trigger on Timer3)
  ADC1->CR = ADC_CR_ADCAL;
  while(ADC1->CR & ADC_CR_ADCAL);
  ADC1->CHSELR = ADC_CHSELR_CHSEL0 | ADC_CHSELR_CHSEL1 | ADC_CHSELR_CHSEL2 | ADC_CHSELR_CHSEL3 | ADC_CHSELR_CHSEL4 | ADC_CHSELR_CHSEL5 | ADC_CHSELR_CHSEL6 | ADC_CHSELR_CHSEL7 | ADC_CHSELR_CHSEL9;
  ADC1->CFGR1 |= ADC_CFGR1_EXTEN_0 | ADC_CFGR1_EXTSEL_1 | ADC_CFGR1_EXTSEL_0 | ADC_CFGR1_DMACFG | ADC_CFGR1_DMAEN;
  ADC1->CR |= ADC_CR_ADEN;
  while(ADC1->ISR & ADC_ISR_ADRDY);
  ADC1->CR |= ADC_CR_ADSTART;
  //Init and Timer3 (20000Hz x 4) for ADC control
  TIM3->ARR = 600 - 1;
  TIM3->CR2 |= TIM_CR2_MMS_1;
  TIM3->CR1 |= TIM_CR1_DIR | TIM_CR1_CEN;
  //Configure SysTicks every 100ms (48MHz Clock)
  SysTick_Config(4800000);
  NVIC_SetPriority(SysTick_IRQn, 2);
  //Configure and init USB
  USB_Init();
  NVIC_EnableIRQ(USB_IRQn);
  NVIC_SetPriority(USB_IRQn, 1);
  USB_Enable(true);
  USB_Connect(true);
}
// Main loop -----------------------------------------------------------------------------------------------------------
void Loop() {
}
//----------------------------------------------------------------------------------------------------------------------