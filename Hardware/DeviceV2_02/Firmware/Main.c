#include <stdbool.h>
#include <stdint.h>
#include <STM32F0xx.h>
#include "USB.h"
// Configuration -------------------------------------------------------------------------------------------------------
#define BUFF_LEN  1200
// USB application definitions -----------------------------------------------------------------------------------------
#define SEND_EP                         0x81
#define RECV_EP                         0x02
#define USB_CONTROL_CLASS_SETSTATUS     0x20
// Types definition ----------------------------------------------------------------------------------------------------
typedef struct {
  uint16_t Power_Color:2;     //Power LED (PB5) Red when off but plugged, off if off and not plugged, green if running and yellow if booting or shuttingdown
  uint16_t Status_Color:2;    //Status LED (PB4) Green if service running, yellow on events (cleared by software), red on service lost communication or errors.
  uint16_t Wifi_Color:2;      //Wifi LED (PB3) Green if connected to internet, yellow if connected to network, red if no connection
  uint16_t Bluetooth_Color:1; //Bluetooth LED (PA9) On when bluetooth enabled
  uint16_t Buzzer:9;          //Buzzer beeps
} SignalsStatus_t;
enum LED_Color {OFF = 0, RED = 1, GREEN = 2, YELLOW = 3 };
typedef struct {
  uint8_t ExtPower:1;   //External power flag
  uint8_t Charging:1;   //Charging flag
  uint8_t Battery:6;    //Battery level
} SysStatus_t;
typedef struct {
  uint16_t Action:2;    //Pending button action
  uint16_t Counter:14;  //Buttons ticks
} ButtonStatus_t;
enum Button_Action { NONE = 0, CLICK = 1, LONG_CLICK = 2, VERY_LONG_CLICK = 3};
typedef struct {
  uint16_t Counter;
  uint16_t Head;
  uint16_t Tail;
  uint16_t Buffer[BUFF_LEN];
} Samples_Buffer_t;
// Local variables -----------------------------------------------------------------------------------------------------
volatile uint32_t SysTicks;
SignalsStatus_t   SignalsStatus = { .Power_Color = OFF, .Status_Color = OFF, .Wifi_Color = OFF, .Buzzer = 0};
SysStatus_t       SysStatus = { .Charging = 0, .ExtPower = 0, .Battery = 0 };
ButtonStatus_t    ButtonStatus = { .Action = CLICK, .Counter = 0};
Samples_Buffer_t  Samples_Buffer = { .Head = 0, .Tail = 0, .Counter = 0 };
uint16_t          ADC_Read[8][9];
// Control LED, buton and buzzer ---------------------------------------------------------------------------------------
void SysTick_ISR() {
  if(Samples_Buffer.Counter == 1000) {
    SignalsStatus.Status_Color = OFF;
    SignalsStatus.Wifi_Color = OFF;
    SignalsStatus.Bluetooth_Color = OFF;
    TIM3->CR1 &= ~TIM_CR1_CEN;
  } else {
    Samples_Buffer.Counter++;
  }
  if((SysTicks % 4) == 0) {
    GPIOA->BSRR = GPIO_BSRR_BS_10 | GPIO_BSRR_BS_15;    //A mode (PA10=1)
    GPIOB->BSRR = GPIO_BSRR_BS_5 | GPIO_BSRR_BS_4;
    GPIOA->BSRR = (SignalsStatus.Bluetooth_Color) ? GPIO_BSRR_BS_9 : GPIO_BSRR_BR_9;
    if(SignalsStatus.Power_Color & RED) GPIOB->BSRR |= GPIO_BSRR_BR_5;
    if(SignalsStatus.Status_Color & RED) GPIOB->BSRR |= GPIO_BSRR_BR_4;
    if(SignalsStatus.Wifi_Color & RED) GPIOA->BSRR |= GPIO_BSRR_BR_15;
  } else {
    GPIOA->BSRR = GPIO_BSRR_BR_10 | GPIO_BSRR_BR_15 | GPIO_BSRR_BR_9;    //A mode (PA10=0)
    GPIOB->BSRR = GPIO_BSRR_BR_5 | GPIO_BSRR_BR_4;
    if(SignalsStatus.Power_Color & GREEN) GPIOB->BSRR |= GPIO_BSRR_BS_5;
    if(SignalsStatus.Status_Color & GREEN) GPIOB->BSRR |= GPIO_BSRR_BS_4;
    if(SignalsStatus.Wifi_Color & GREEN) GPIOA->BSRR |= GPIO_BSRR_BS_15;
  }
  if((SysTicks % 64) == 0) {
    TIM2->CCR2 = ((SignalsStatus.Buzzer & 0x001) ? 1000 - 1 : 0);
    SignalsStatus.Buzzer = SignalsStatus.Buzzer >> 1;    
  }
  //Detecto pulsacion de boton
  if(ButtonStatus.Action == NONE) {
    if(GPIOA->IDR & GPIO_IDR_0) {
      if(ButtonStatus.Counter == 0) SignalsStatus.Buzzer = 0b000000001;
      ButtonStatus.Counter++;
      if(ButtonStatus.Counter == 1000) SignalsStatus.Buzzer = 0b000000001;
      if(ButtonStatus.Counter == 5000) SignalsStatus.Buzzer = 0b000000101;
      if(ButtonStatus.Counter > 10000) {
        ButtonStatus.Action = VERY_LONG_CLICK;
        SignalsStatus.Buzzer = 0b101010111;
        ButtonStatus.Counter = 0;
      }
    } else if(ButtonStatus.Counter > 5000) {
      ButtonStatus.Action = LONG_CLICK;
      SignalsStatus.Buzzer = 0b001010111;
      ButtonStatus.Counter = 0;
    } else if(ButtonStatus.Counter > 200) {
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
  uint16_t *Ptr;
  if(DMA1->ISR & DMA_ISR_HTIF1) {
    Ptr = ADC_Read[0];
    DMA1->IFCR |= DMA_IFCR_CHTIF1;
  } else {
    Ptr = ADC_Read[4];
    DMA1->IFCR |= DMA_IFCR_CTCIF1;
  }
  int16_t Offset = (2048 * 4) - (Ptr[0] + Ptr[9] + Ptr[18] + Ptr[27]);
  if((Offset > 256) || (Offset < -256)) Offset = 0;
  for(uint8_t Count = 0; Count < 7; Count++) {
    uint16_t Next = (Samples_Buffer.Head + 1) % BUFF_LEN;
    Ptr++;
    if(Next != Samples_Buffer.Tail) {
      Samples_Buffer.Buffer[Samples_Buffer.Head] = ((Ptr[0] + Ptr[9] + Ptr[18] + Ptr[27] + Offset) >> 1) | (Count << 13);
      Samples_Buffer.Head = Next;
    } else {
      Samples_Buffer.Head = 0;
      Samples_Buffer.Tail = 0;
//     __asm("BKPT #0\n") ; // Break into the debugger
    }
  }
  //const uint16_t Temp30 = *(uint16_t*)0x1FFFF7B8;
  //const uint16_t Temp110 = *(uint16_t*)0x1FFFF7C2;
  //int32_t Temp = ((Ptr[0].Temperature - Temp30) * 80) / (Temp110 - Temp30) + 30;
  //TIM16->CCR1 = (Temp < 50) ? 0 : Temp * 30000;
  if(Ptr[8] < 2560) {
    SysStatus.Battery = 0;
  } else {
    Ptr[8] = Ptr[8] - 2560 / 18;
    SysStatus.Battery = (Ptr[8] > 63) ? 63 : Ptr[8];
  }
//  int32_t Battery = ((uint32_t)Ptr[8] * 4520) / 4096;
//  SysStatus.Battery = (Battery <= 2960) ? 0 : (Battery >= 4240) ? 63 : (Battery - 2960) / 20;
}
// USB callbacks functions ---------------------------------------------------------------------------------------------
void USB_HandleEP1_TX(uint8_t event) {
  uint16_t fifo[32];
  if(event != USB_EVENT_TX) return;
  for(int x = 0; x < 32; x++) {
    if(Samples_Buffer.Head == Samples_Buffer.Tail) {
      fifo[x] = 0xE000 | (ButtonStatus.Action << 8) | (SysStatus.Battery << 2) | (SysStatus.Charging << 1) | (SysStatus.ExtPower);
    } else {
      uint16_t Val = Samples_Buffer.Buffer[Samples_Buffer.Tail];
      Samples_Buffer.Tail = (Samples_Buffer.Tail + 1) % BUFF_LEN;
      fifo[x] = Val;
    }
  }
  USB_WriteEP(SEND_EP, fifo, 64);
}
void USB_HandleEP2_RX(uint8_t event) {
  if(event != USB_EVENT_RX) return;
  uint16_t Read;
  USB_ReadEP(RECV_EP, &Read, 2);
  //0-1: Status_Color
  SignalsStatus.Status_Color    = (Read >> 0) & 0x0003;
  //2-3: Wifi_Color
  SignalsStatus.Wifi_Color      = (Read >> 2) & 0x0003;
  //4: Bluetooth_Color
  SignalsStatus.Bluetooth_Color = (Read >> 4) & 0x0001;
  //5-13: Buzzer patern
  SignalsStatus.Buzzer          = (Read >> 5) & 0x01FF;
  Samples_Buffer.Counter = 0;
  //14: Gain bit
  GPIOF->BSRR |= ((Read >> 14) & 0x0001) ? GPIO_BSRR_BS_1 : GPIO_BSRR_BR_1;
  TIM3->CR1 |= TIM_CR1_CEN;
}
USB_Response USB_SetConfiguration(uint8_t cfg) {
  switch(cfg) {
    case 0:
      USB_DeconfigEP(SEND_EP);
      USB_RegisterEP(SEND_EP, 0);
      USB_DeconfigEP(RECV_EP);
      USB_RegisterEP(RECV_EP, 0);
      return USB_ACK;
    case 1:
      USB_ConfigEP(SEND_EP, USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF, 64);
      USB_RegisterEP(SEND_EP, USB_HandleEP1_TX);
      USB_ConfigEP(RECV_EP, USB_EPTYPE_BULK, 2);
      USB_RegisterEP(RECV_EP, USB_HandleEP2_RX);
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
  RCC->APB1ENR |= RCC_APB1ENR_USBEN | RCC_APB1ENR_CRSEN | RCC_APB1ENR_PWREN | RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN;
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN | RCC_APB2ENR_ADC1EN | RCC_APB2ENR_TIM16EN;
  CRS->CR |= CRS_CR_AUTOTRIMEN | CRS_CR_CEN;
  //Configure port F (PF1 -> Digital outputs)
  GPIOF->MODER   |= GPIO_MODER_MODER1_0;
  GPIOF->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR1;
  //Configure Port A (PA0, PA8 -> Digital inputs, PA9, PA10, PA15 -> Digital outputs, PA1, PA2, PA3, PA4, PA5, PA6, PA7 -> Analog input, PA11, PA12 -> Alternate function, PA13, PA14 -> SWD)
  GPIOA->MODER |= GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7 | GPIO_MODER_MODER9_0 | 
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
  // Initialize the ADC (14MHz, 1.5 adquisition, DMA active, trigger on Timer3)
  ADC1->CR = ADC_CR_ADCAL;
  while(ADC1->CR & ADC_CR_ADCAL);
  ADC1->CHSELR = ADC_CHSELR_CHSEL1 | ADC_CHSELR_CHSEL2 | ADC_CHSELR_CHSEL3 | ADC_CHSELR_CHSEL4 | ADC_CHSELR_CHSEL5 | ADC_CHSELR_CHSEL6 | ADC_CHSELR_CHSEL7 | ADC_CHSELR_CHSEL8 | ADC_CHSELR_CHSEL9;
  ADC1->CFGR1 |= ADC_CFGR1_EXTEN_0 | ADC_CFGR1_EXTSEL_1 | ADC_CFGR1_EXTSEL_0 | ADC_CFGR1_DMACFG | ADC_CFGR1_DMAEN;
  ADC1->CR |= ADC_CR_ADEN;
  while(ADC1->ISR & ADC_ISR_ADRDY);
  ADC1->CR |= ADC_CR_ADSTART;
  //Init and Timer3 (19200Hz x 4) for ADC control
  TIM3->ARR = 500 - 1;
  TIM3->CR2 |= TIM_CR2_MMS_1;
  TIM3->CR1 |= TIM_CR1_DIR;
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
  TIM16->CCR1 = 900;
  //Configure SysTicks every 1ms (48MHz Clock)
  SysTick_Config(48000);
  NVIC_SetPriority(SysTick_IRQn, 2);
  // Configure and init USB 
  USB_Init();
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
  static uint8_t Charging = 0;
  //Read charging and external power bits
  SysStatus.ExtPower = (GPIOB->IDR & GPIO_IDR_7) ? 1 : 0;
  if(GPIOA->IDR & GPIO_IDR_8) {
    Charging++;
    if(Charging == 0xFF) SysStatus.Charging = 1;
  } else {
    SysStatus.Charging = 0;
  }
  if(ButtonStatus.Action == CLICK) {
    //Handle power on action
    if((GPIOB->ODR && GPIO_ODR_6) == 0) {
      SignalsStatus.Power_Color = RED;
      Sleep(1000);
      SignalsStatus.Power_Color = YELLOW;
      SignalsStatus.Buzzer = 0b111010101;
      GPIOB->BSRR |= GPIO_BSRR_BS_6;
      //Wait NanoPI up to 20 sec
      uint32_t Time = SysTicks + 20000;
      while(SysTicks < Time) {
        if(USB_Device.Status.device_state == USB_STATE_CONFIGURED) {
          ButtonStatus.Action = NONE;
          SignalsStatus.Power_Color = GREEN;
          SignalsStatus.Buzzer = 0b111101111;
          return;
        }
      }
      SignalsStatus.Buzzer = 0b000001111;
      SignalsStatus.Power_Color = RED;
      GPIOB->BSRR |= GPIO_BSRR_BR_6;
      Sleep(1000);
    } else {
     Sleep(2000);
     ButtonStatus.Action = NONE;
    }
  } else if(ButtonStatus.Action == LONG_CLICK) {
    SignalsStatus.Power_Color = YELLOW;
    //Wait NanoPI to shutdown up to 20 sec
    uint32_t Time = SysTicks + 20000;
    while(SysTicks < Time) {
      if(USB->CNTR & USB_CNTR_FSUSP) {
        SignalsStatus.Buzzer = 0b000001111;
        SignalsStatus.Power_Color = RED;
        Sleep(1000);
        Suspend();
      }
    }
    SignalsStatus.Power_Color = GREEN;
    ButtonStatus.Action = NONE;    
  } else if(ButtonStatus.Action == VERY_LONG_CLICK) {
    Sleep(2000);
    Suspend();
  }
}
