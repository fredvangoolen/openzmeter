#include <stdbool.h>
#include <stdint.h>
#include <STM32F0xx.h>
#include "USB.h"
// Configuration -------------------------------------------------------------------------------------------------------
#define BUFF_LEN 1024
// Types definition ----------------------------------------------------------------------------------------------------
typedef struct {
  uint16_t Offset;
  uint16_t V_CH_1;
  uint16_t V_CH_2;
  uint16_t V_CH_3;
  uint16_t I_CH_1;
  uint16_t I_CH_2;
  uint16_t I_CH_3;
  uint16_t I_CH_4;
  uint16_t Battery;
} ADC_Sequence_t;
typedef struct {
  uint16_t Power_Color:2;     //Power LED (PB5) Red when off but plugged, off if off and not plugged, green if running and yellow if booting or shuttingdown
  uint16_t Status_Color:2;    //Status LED (PB4) Green if service running, yellow on events (cleared by software), red on service lost communication or errors.
  uint16_t Wifi_Color:2;      //Wifi LED (PB3) Green if connected to internet, yellow if connected to network, red if no connection
  uint16_t Bluetooth_Color:1; //Bluetooth LED (PA9) On when bluetooth enabled
  uint16_t Buzzer:9;          //Buzzer beeps
} SignalsStatus_t;
enum LED_Color {OFF = 0, RED = 1, GREEN = 2, YELLOW = 3 };
typedef struct {
  uint8_t ExtPower:1;         //External power flag
  uint8_t Charging:1;         //Charging flag
  uint8_t Battery:6;          //Battery level
} SysStatus_t;
typedef struct {
  uint16_t Action:2;           //Pending button action
  uint16_t Counter:14;  //Buttons ticks
} ButtonStatus_t;
enum Button_Action { NONE = 0, CLICK = 1, LONG_CLICK = 2, VERY_LONG_CLICK = 3};
typedef struct {
  uint16_t Head;
  uint16_t Tail;
  uint16_t Buffer[BUFF_LEN];
} USB_Buffer_t;
// Local variables -----------------------------------------------------------------------------------------------------
volatile uint32_t SysTicks;
SignalsStatus_t   SignalsStatus = { .Power_Color = OFF, .Status_Color = OFF, .Wifi_Color = OFF, .Buzzer = 0};
SysStatus_t       SysStatus = { .Charging = 0, .ExtPower = 0, .Battery = 0 };
ButtonStatus_t    ButtonStatus = { .Action = CLICK, .Counter = 0};
USB_Buffer_t      USB_Buffer = { .Head = 0, .Tail = 0 };
ADC_Sequence_t    ADC_Read[8];
// Buffer management functions -----------------------------------------------------------------------------------------
void BufferPush(uint16_t value) {
  uint16_t Next = (USB_Buffer.Head + 1) % BUFF_LEN;
  if(Next != USB_Buffer.Tail) {
    USB_Buffer.Buffer[USB_Buffer.Head] = value;
    USB_Buffer.Head = Next;
  } else {
    USB_Buffer.Head = 0;
    USB_Buffer.Tail = 0;
//    USB_Buffer.Size = 0;
//   __asm("BKPT #0\n") ; // Break into the debugger
  }  
}
uint16_t BufferPop() {
  if(USB_Buffer.Head == USB_Buffer.Tail) {
    return 0xE000 | (ButtonStatus.Action << 8) | (SysStatus.Battery << 2) | (SysStatus.Charging << 1) | (SysStatus.ExtPower);
  } else {
    uint16_t Val = USB_Buffer.Buffer[USB_Buffer.Tail];
    USB_Buffer.Tail = (USB_Buffer.Tail + 1) % BUFF_LEN;
    return Val;
  }
}
// Control LED, buton and buzzer ---------------------------------------------------------------------------------------
void SysTick_ISR() {
  if(SysTicks % 2) {
    GPIOA->BSRR = GPIO_BSRR_BS_10 | GPIO_BSRR_BS_15;    //A mode (PA10=1)
    GPIOB->BSRR = GPIO_BSRR_BS_5 | GPIO_BSRR_BS_4;
    if(((SysTicks % 8) == 1) && ((SignalsStatus.Power_Color & GREEN) == GREEN)) GPIOB->BSRR |= GPIO_BSRR_BR_5;
    if(((SysTicks % 8) == 3) && ((SignalsStatus.Status_Color & GREEN) == GREEN)) GPIOB->BSRR |= GPIO_BSRR_BR_4;
    if(((SysTicks % 8) == 5) && ((SignalsStatus.Wifi_Color & GREEN) == GREEN)) GPIOA->BSRR |= GPIO_BSRR_BR_15;
  } else {
    GPIOA->BSRR = GPIO_BSRR_BR_10 | GPIO_BSRR_BR_15;    //A mode (PA10=0)
    GPIOB->BSRR = GPIO_BSRR_BR_5 | GPIO_BSRR_BR_4;
    if(((SysTicks % 8) == 0) && ((SignalsStatus.Power_Color & RED) == RED)) GPIOB->BSRR |= GPIO_BSRR_BS_5;
    if(((SysTicks % 8) == 2) && ((SignalsStatus.Status_Color & RED) == RED)) GPIOB->BSRR |= GPIO_BSRR_BS_4;
    if(((SysTicks % 8) == 4) && ((SignalsStatus.Wifi_Color & RED) == RED)) GPIOA->BSRR |= GPIO_BSRR_BS_15;
  }
  GPIOA->BSRR = (SignalsStatus.Bluetooth_Color) ? GPIO_BSRR_BS_9 : GPIO_BSRR_BR_9;
  if((SysTicks % 64) == 0) {
    TIM2->CCR2 = ((SignalsStatus.Buzzer & 0x001) ? 1000 - 1 : 0);
    SignalsStatus.Buzzer = SignalsStatus.Buzzer >> 1;    
  }
  //Detecto pulsacion de boton
  if(ButtonStatus.Action == NONE) {
    if(GPIOA->IDR & GPIO_IDR_0) {
      if(ButtonStatus.Counter == 0) SignalsStatus.Buzzer = 0b000000001;
      ButtonStatus.Counter++;
      if(ButtonStatus.Counter > 10000) {
        ButtonStatus.Action = VERY_LONG_CLICK;
        SignalsStatus.Buzzer = 0b101010111;
        ButtonStatus.Counter = 0;
      }
    } else if(ButtonStatus.Counter > 5000) {
      ButtonStatus.Action = LONG_CLICK;
      SignalsStatus.Buzzer = 0b001010111; 
      ButtonStatus.Counter = 0;
    } else if(ButtonStatus.Counter > 1000) {
      ButtonStatus.Action = CLICK;
      SignalsStatus.Buzzer = 0b000010111;
      ButtonStatus.Counter = 0;
    } else {
      ButtonStatus.Counter = 0;    
    }
  }
  SysTicks++;
}
// Suspend device and prepare for later wake up on button press --------------------------------------------------------
void Suspend() {
  PWR->CSR |= PWR_CSR_EWUP1;
  PWR->CR |= PWR_CR_CWUF | PWR_CR_CSBF | PWR_CR_PDDS | PWR_CR_LPDS;
  SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  __DSB();
  __WFI();
}
// DMA interrupts to handle ADC oversample -----------------------------------------------------------------------------
void DMA_CH1_ISR() {
  ADC_Sequence_t *Ptr = (DMA1->ISR & DMA_ISR_HTIF1) ? &ADC_Read[0] : &ADC_Read[4];
  DMA1->IFCR |= DMA_IFCR_CTCIF1 | DMA_IFCR_CHTIF1;
  int16_t Offset = (2048 * 4) - (Ptr[0].Offset + Ptr[1].Offset + Ptr[2].Offset + Ptr[3].Offset);
  if((Offset > 256) || (Offset < -256)) Offset = 0;
  BufferPush(((Ptr[0].V_CH_1 + Ptr[1].V_CH_1 + Ptr[2].V_CH_1 + Ptr[3].V_CH_1 + Offset) >> 1) | 0x0000);
  BufferPush(((Ptr[0].V_CH_2 + Ptr[1].V_CH_2 + Ptr[2].V_CH_2 + Ptr[3].V_CH_2 + Offset) >> 1) | 0x2000);
  BufferPush(((Ptr[0].V_CH_3 + Ptr[1].V_CH_3 + Ptr[2].V_CH_3 + Ptr[3].V_CH_3 + Offset) >> 1) | 0x4000);
  BufferPush(((Ptr[0].I_CH_1 + Ptr[1].I_CH_1 + Ptr[2].I_CH_1 + Ptr[3].I_CH_1 + Offset) >> 1) | 0x6000);
  BufferPush(((Ptr[0].I_CH_2 + Ptr[1].I_CH_2 + Ptr[2].I_CH_2 + Ptr[3].I_CH_2 + Offset) >> 1) | 0x8000);
  BufferPush(((Ptr[0].I_CH_3 + Ptr[1].I_CH_3 + Ptr[2].I_CH_3 + Ptr[3].I_CH_3 + Offset) >> 1) | 0xA000);
  BufferPush(((Ptr[0].I_CH_4 + Ptr[1].I_CH_4 + Ptr[2].I_CH_4 + Ptr[3].I_CH_4 + Offset) >> 1) | 0xC000);
  //const uint16_t Temp30 = *(uint16_t*)0x1FFFF7B8;
  //const uint16_t Temp110 = *(uint16_t*)0x1FFFF7C2;
//  int32_t Temp = ((Ptr[0].Temperature - Temp30) * 80) / (Temp110 - Temp30) + 30;
//  TIM16->CCR1 = (Temp < 50) ? 0 : Temp * 30000;
  int32_t Battery = (Ptr[0].Battery * 4522) / 4096;
  SysStatus.Battery = (Battery <= 2960) ? 0 : (Battery >= 4240) ? 63 : (Battery - 2960) / 20;
}
// USB callbacks functions ---------------------------------------------------------------------------------------------
//static void cdc_rxonly (usbd_device *dev, uint8_t event, uint8_t ep) {
//   usbd_ep_read(dev, ep, fifo, CDC_DATA_SZ);
//}
uint16_t Serie = 0;
uint16_t fifo[32];
void cdc_txonly(uint8_t event, uint8_t ep) {
//    uint8_t _t = dev->driver->frame_no();
//    memset(fifo, _t, CDC_DATA_SZ);
//  if(USB_Buffer.Size < 31) {
//    ep_write(ep, fifo, 0);
//    return;
//  }
  for(int x = 0; x < 32; x++) {
    fifo[x] = BufferPop();
  }
  USB_EP_Write(ep, fifo, 64);
}

void SendBuffer() {
  // Copio desde el buffer y voy reduciendo, si no hay muestras que enviar relleno con FFFF para que no se bloquee el EP
  // Cada 100 bloques enviados mando estado

}
void SetState() {
  //Configure LEDs state and beeps
  //Configure gains 
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
  RCC->APB1ENR |= RCC_APB1ENR_USBEN | RCC_APB1ENR_CRSEN | RCC_APB1ENR_PWREN | RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN;
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN | RCC_APB2ENR_ADC1EN | RCC_APB2ENR_TIM16EN;
  CRS->CR |= CRS_CR_AUTOTRIMEN | CRS_CR_CEN;
  //Configure port F (PF0, PF1 -> Digital outputs)
  GPIOF->MODER   |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0;
  GPIOF->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR1;
  //Configure Port A (PA0, PA8 -> Digital inputs, PA1, PA9, PA10, PA15 -> Digital outputs, PA2, PA3, PA4, PA5, PA6, PA7 -> Analog input, PA11, PA12 -> Alternate function, PA13, PA14 -> SWD)
  GPIOA->MODER |= GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7 | GPIO_MODER_MODER9_0 | 
                  GPIO_MODER_MODER10_0 | GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1 | GPIO_MODER_MODER15_0;
  GPIOA->PUPDR |= GPIO_PUPDR_PUPDR8_0 | GPIO_PUPDR_PUPDR0_1;
  //Configure Port B (PB7 -> Digital input, PB0, PB1 -> Analog inputs, PB4, PB5, PB6 -> Digital output, PB3, PB8 -> Alternate function)
  GPIOB->MODER |= GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER3_1 | GPIO_MODER_MODER8_1 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 | GPIO_MODER_MODER6_0;
  GPIOB->AFR[0] |= 0x00002000; //PB3 -> TIM2_CH2
  GPIOB->AFR[1] |= 0x00000002; //PB8 -> TIM16_CH1
  //Enable DMA controller
  DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
  DMA1_Channel1->CMAR = (uint32_t)ADC_Read;
  DMA1_Channel1->CNDTR = 72;
  DMA1_Channel1->CCR |= DMA_CCR_PL_0 | DMA_CCR_PL_1 | DMA_CCR_PSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_MSIZE_0 | DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_EN;
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
  // Initialize the ADC (14MHz, 7.5 adquisition, DMA active, trigger on Timer3)
//  ADC->CCR |= ADC_CCR_TSEN;
  ADC1->SMPR |= ADC_SMPR_SMP_0;
//  ADC1->CFGR2 |= ADC_CFGR2_CKMODE_1;
  ADC1->CR = ADC_CR_ADCAL;
  while(ADC1->CR & ADC_CR_ADCAL);
  ADC1->CHSELR = ADC_CHSELR_CHSEL1 | ADC_CHSELR_CHSEL2 | ADC_CHSELR_CHSEL3 | ADC_CHSELR_CHSEL4 | ADC_CHSELR_CHSEL5 | ADC_CHSELR_CHSEL6 | ADC_CHSELR_CHSEL7 | ADC_CHSELR_CHSEL8 | ADC_CHSELR_CHSEL9;
  ADC1->CFGR1 |= ADC_CFGR1_EXTEN_0 | ADC_CFGR1_EXTSEL_1 | ADC_CFGR1_EXTSEL_0 | ADC_CFGR1_DMACFG | ADC_CFGR1_DMAEN;
  ADC1->CR |= ADC_CR_ADEN;
  while(ADC1->ISR & ADC_ISR_ADRDY);
  ADC1->CR |= ADC_CR_ADSTART;
  //Init and Timer3 (19200Hz x 4) for ADC control
  TIM3->ARR = 625 - 1;
  TIM3->CR2 |= TIM_CR2_MMS_1;
  TIM3->CR1 |= TIM_CR1_DIR | TIM_CR1_CEN;
  // Configure Timer2 (4kHz 50%) for sounder
  TIM2->CCMR1 |= TIM_CCMR1_OC2PE | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;
  TIM2->ARR = 12000 - 1;
  TIM2->CCER  |= TIM_CCER_CC2E; // BEEPER_TIM_CH2 output compare enable
  TIM2->EGR    = TIM_EGR_UG; // Generate an update event to reload the prescaler value immediately
  TIM2->CR1   |= TIM_CR1_ARPE | TIM_CR1_CEN;
  // Configure Timer16 (20kHz) for fan
  TIM16->CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
  TIM16->ARR    = 1200 - 1;
  TIM16->CCER  |= TIM_CCER_CC1E; // FAN_TIM_CH1 output compare enable
  TIM16->EGR    = TIM_EGR_UG; // Generate an update event to reload the prescaler value immediately
  TIM16->CR1   |= TIM_CR1_ARPE | TIM_CR1_CEN;
  TIM16->BDTR  |= TIM_BDTR_MOE;
  TIM16->CCR1 = 800;
  //Configure SysTicks every 1ms (48MHz Clock)
  SysTick_Config(48000);
  NVIC_SetPriority(SysTick_IRQn, 2);
  // Configure and init USB 
  usbd_init();
  //cdc_init_usbd();
  NVIC_EnableIRQ(USB_IRQn);
  NVIC_SetPriority(USB_IRQn, 1);
  USB_Enable(true);
  USB_Connect(true);
}
void Sleep(uint16_t ms) {
  uint32_t Time = SysTicks + ms;
  while(SysTicks < Time);
} 
// Main loop -----------------------------------------------------------------------------------------------------------
void Loop() {
  //Read charging and external power bits
  SysStatus.ExtPower = (GPIOB->IDR & GPIO_IDR_7) ? 1 : 0;
  SysStatus.Charging = (GPIOA->IDR & GPIO_IDR_8) ? 0 : 1;
  //Handle power on action
  if(((GPIOB->ODR && GPIO_ODR_6) == 0) && (ButtonStatus.Action == CLICK)) {
    SignalsStatus.Power_Color = RED;
    Sleep(1000);
    SignalsStatus.Power_Color = YELLOW;
    SignalsStatus.Buzzer = 0b111010101;
    GPIOB->BSRR |= GPIO_BSRR_BS_6;
    //Wait NanoPI up to 20 sec
    uint32_t Time = SysTicks + 20000;
    while(SysTicks < Time) {
      if(udev.Status.device_state == USB_STATE_CONFIGURED) {
        ButtonStatus.Action = NONE;
        SignalsStatus.Power_Color = GREEN;
        SignalsStatus.Buzzer = 0b111101111;
        return;
      }
    }
    SignalsStatus.Buzzer = 0b000001111;
    SignalsStatus.Power_Color = RED;
    Sleep(1000);
    GPIOB->BSRR |= GPIO_BSRR_BR_6;
  }
  if(((GPIOB->ODR && GPIO_ODR_6) == 1) && (ButtonStatus.Action == CLICK)) {
    //Envio pulsacion corta a la nanopi
  }
  if(((GPIOB->ODR && GPIO_ODR_6) == 1) && (ButtonStatus.Action == LONG_CLICK)) {
    //Envio pulsacion corta a la nanopi
  }
  if(((GPIOB->ODR && GPIO_ODR_6) == 1) && (ButtonStatus.Action == VERY_LONG_CLICK)) {
    Sleep(2000);
    Suspend();
  }
}