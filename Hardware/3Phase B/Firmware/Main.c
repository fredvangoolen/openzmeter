#include <stdbool.h>
#include <stdint.h>
#include <STM32F0xx.h>
#include "USB.h"
#define BUFF_LEN 1024
// Types definition ----------------------------------------------------------------------------------------------------
typedef struct {
  uint16_t VR;
  uint16_t VS;
  uint16_t VT;
  uint16_t IR;
  uint16_t IS;
  uint16_t IN;
  uint16_t IT;
  uint16_t Battery;
} ADC_Sequence_t;
typedef struct {
  uint8_t Prescaler;        //Increases on each interrupt event
} SysTicks_t;
typedef struct {
  uint8_t Battery;            //Battery level
  uint8_t Buzzer:2;           //Buzzer beeps
  uint8_t Button:2;           //Pending button action
  uint8_t Power_A_ON:1;       //Power LED (PB5)
  uint8_t Power_A_BLINK:1;
  uint8_t Power_B_ON:1;
  uint8_t Power_B_BLINK:1;
  uint8_t Status_A_ON:1;      //Status LED (PB4)
  uint8_t Status_A_BLINK:1;
  uint8_t Status_B_ON:1;
  uint8_t Status_B_BLINK:1;
  uint8_t Wifi_A_ON:1;        //Wifi LED (PB3)
  uint8_t Wifi_A_BLINK:1;
  uint8_t Wifi_B_ON:1;
  uint8_t Wifi_B_BLINK:1;
  uint8_t Bluetooth_ON:1;     //Bluetooth LED (PA9)
  uint8_t Bluetooth_BLINK:1;
  uint8_t On:1;               //System is on (5v boost on)
  uint8_t Ready:1;            //NanoPI is active (yet booting or shuting down)
  uint8_t ExtPower:1;         //External power flag
  uint8_t Charging:1;         //Charging flag
  //Me quedan 2 bits
} SystemStatus_t;
typedef struct {
  uint16_t Buffer[BUFF_LEN];
  uint16_t Head;
  uint16_t Tail;
//  uint16_t Size;  
} USB_Buffer_t;
// Local variables -----------------------------------------------------------------------------------------------------
SystemStatus_t SysStatus;
SysTicks_t     SysTicks;
ADC_Sequence_t ADC_Read[8];
USB_Buffer_t   USB_Buffer;
// Buffer management functions -----------------------------------------------------------------------------------------
void Buffer_Init() {
  USB_Buffer.Head = 0;
  USB_Buffer.Tail = 0;
//  USB_Buffer.Size = 0;
}
void BufferPush(uint16_t value) {
  uint16_t Next = (USB_Buffer.Head + 1) % BUFF_LEN;
  if(Next != USB_Buffer.Tail) {
    USB_Buffer.Buffer[USB_Buffer.Head] = value;
    USB_Buffer.Head = Next;
  //if(USB_Buffer.Size < BUFF_LEN) USB_Buffer.Size++;
  } else {
//    USB_Buffer.Size = 0;
//   __asm("BKPT #0\n") ; // Break into the debugger
  }  
}
uint16_t BufferPop() {
  if(USB_Buffer.Head == USB_Buffer.Tail) {
    return 0xFFFF;
  } else {
    uint16_t Val = USB_Buffer.Buffer[USB_Buffer.Tail];
    USB_Buffer.Tail = (USB_Buffer.Tail + 1) % BUFF_LEN;
    return Val;
  }
}
// Control LED, buton and buzzer ---------------------------------------------------------------------------------------
void SysTick_ISR() {
  uint32_t Out = 0;
  if(SysTicks.Prescaler % 2) {
    GPIOA->BSRR = GPIO_BSRR_BS_10;    //A mode (PA10=1)
    GPIOB->BSRR = GPIO_BSRR_BS_5 | GPIO_BSRR_BS_4 | GPIO_BSRR_BS_3;
    if(((SysTicks.Prescaler % 4) == 0) && SysStatus.Power_A_ON  && (!SysStatus.Power_A_BLINK  || (SysStatus.Power_A_BLINK  && (SysTicks.Prescaler < 127)))) Out |= GPIO_BSRR_BR_5;
    if(((SysTicks.Prescaler % 4) == 1) && SysStatus.Status_A_ON && (!SysStatus.Status_A_BLINK || (SysStatus.Status_A_BLINK && (SysTicks.Prescaler < 127)))) Out |= GPIO_BSRR_BR_4;
    if(((SysTicks.Prescaler % 4) == 2) && SysStatus.Wifi_A_ON   && (!SysStatus.Wifi_A_BLINK   || (SysStatus.Wifi_A_BLINK   && (SysTicks.Prescaler < 127)))) Out |= GPIO_BSRR_BR_3;
  } else {
    GPIOA->BSRR = GPIO_BSRR_BR_10;    //A mode (PA10=0)
    GPIOB->BSRR = GPIO_BSRR_BR_5 | GPIO_BSRR_BR_4 | GPIO_BSRR_BR_3;
    if(((SysTicks.Prescaler % 4) == 0) && SysStatus.Power_B_ON  && (!SysStatus.Power_B_BLINK  || (SysStatus.Power_B_BLINK  && (SysTicks.Prescaler < 127)))) Out |= GPIO_BSRR_BS_5;
    if(((SysTicks.Prescaler % 4) == 1) && SysStatus.Status_B_ON && (!SysStatus.Status_B_BLINK || (SysStatus.Status_B_BLINK && (SysTicks.Prescaler < 127)))) Out |= GPIO_BSRR_BS_4;
    if(((SysTicks.Prescaler % 4) == 2) && SysStatus.Wifi_B_ON   && (!SysStatus.Wifi_B_BLINK   || (SysStatus.Wifi_B_BLINK   && (SysTicks.Prescaler < 127)))) Out |= GPIO_BSRR_BS_3;
  }
  GPIOA->BSRR = (((SysTicks.Prescaler % 8) == 0) && SysStatus.Bluetooth_ON && (!SysStatus.Bluetooth_BLINK || (SysStatus.Bluetooth_BLINK && (SysTicks.Prescaler < 127)))) ? GPIO_BSRR_BS_9 : GPIO_BSRR_BR_9;
  GPIOB->BSRR = Out;
  //Detecto pulsacion de boton
  
  SysTicks.Prescaler++;
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
  GPIOA->ODR =  GPIOA->ODR ^ GPIO_ODR_1;
  ADC_Sequence_t *Ptr = (DMA1->ISR & DMA_ISR_HTIF1) ? &ADC_Read[0] : &ADC_Read[4];
  DMA1->IFCR |= DMA_IFCR_CTCIF1 | DMA_IFCR_CHTIF1;
  BufferPush(((Ptr[0].VR + Ptr[1].VR + Ptr[2].VR + Ptr[3].VR) >> 1) | 0x0000);
  BufferPush(((Ptr[0].VS + Ptr[1].VS + Ptr[2].VS + Ptr[3].VS) >> 1) | 0x2000);
  BufferPush(((Ptr[0].VT + Ptr[1].VT + Ptr[2].VT + Ptr[3].VT) >> 1) | 0x4000);
  BufferPush(((Ptr[0].IR + Ptr[1].IR + Ptr[2].IR + Ptr[3].IR) >> 1) | 0x6000);
  BufferPush(((Ptr[0].IS + Ptr[1].IS + Ptr[2].IS + Ptr[3].IS) >> 1) | 0x8000);
  BufferPush(((Ptr[0].IT + Ptr[1].IT + Ptr[2].IT + Ptr[3].IT) >> 1) | 0xA000);
  BufferPush(((Ptr[0].IN + Ptr[1].IN + Ptr[2].IN + Ptr[3].IN) >> 1) | 0xC000);
  SysStatus.Battery = Ptr[0].Battery >> 4;
}
// USB callbacks functions ---------------------------------------------------------------------------------------------
//static void cdc_rxonly (usbd_device *dev, uint8_t event, uint8_t ep) {
//   usbd_ep_read(dev, ep, fifo, CDC_DATA_SZ);
//}
uint16_t Serie = 0;
uint16_t fifo[32];
void cdc_txonly(usbd_device *dev, uint8_t event, uint8_t ep) {
//    uint8_t _t = dev->driver->frame_no();
//    memset(fifo, _t, CDC_DATA_SZ);
//  if(USB_Buffer.Size < 31) {
//    ep_write(ep, fifo, 0);
//    return;
//  }
  for(int x = 0; x < 32; x++) {
    fifo[x] = BufferPop();
  }
  ep_write(ep, fifo, 64);
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
  RCC->APB1ENR |= RCC_APB1ENR_USBEN | RCC_APB1ENR_CRSEN | RCC_APB1ENR_PWREN | RCC_APB1ENR_TIM3EN;
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN | RCC_APB2ENR_ADC1EN;
  //Configure port F (PF0, PF1 -> Digital outputs)
  GPIOF->MODER   |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0;
  GPIOF->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR1;
  //Configure Port A (PA0, PA8 -> Digital inputs, PA1, PA9, PA10, PA15 -> Digital outputs, PA2, PA3, PA4, PA5, PA6, PA7 -> Analog input, PA11, PA12 -> Alternate function, PA13, PA14 -> SWD)
  GPIOA->MODER |= GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7 | GPIO_MODER_MODER9_0 | 
                  GPIO_MODER_MODER10_0 | GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1 | GPIO_MODER_MODER15_0;
  //Configure Port B (PB7 -> Digital input, PB0, PB1 -> Analog inputs, PB3, PB4, PB5, PB6)
  GPIOB->MODER |= GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 | GPIO_MODER_MODER6_0;
  //Enable DMA controller
  DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
  DMA1_Channel1->CMAR = (uint32_t)ADC_Read;
  DMA1_Channel1->CNDTR = 64;
  DMA1_Channel1->CCR |= DMA_CCR_PL_0 | DMA_CCR_PL_1 | DMA_CCR_PSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_MSIZE_0 | DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_EN;
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
  // Initialize the ADC (12MHz, 7.5 adquisition, DMA active, trigger on Timer3)
  ADC1->CFGR1 |= ADC_CFGR1_EXTEN_0 | ADC_CFGR1_EXTSEL_1 | ADC_CFGR1_EXTSEL_0 | ADC_CFGR1_DMACFG | ADC_CFGR1_DMAEN;
//  ADC1->CFGR2 |= ADC_CFGR2_CKMODE_1;
  ADC1->SMPR |= ADC_SMPR_SMP_0;
  ADC1->CHSELR |= ADC_CHSELR_CHSEL2 | ADC_CHSELR_CHSEL3 | ADC_CHSELR_CHSEL4 | ADC_CHSELR_CHSEL5 | ADC_CHSELR_CHSEL6 | ADC_CHSELR_CHSEL7 | ADC_CHSELR_CHSEL8 | ADC_CHSELR_CHSEL9 ;
  ADC1->CR = ADC_CR_ADCAL;
  while(ADC1->CR & ADC_CR_ADCAL);
  ADC1->CR |= ADC_CR_ADEN;
  while(ADC1->ISR & ADC_ISR_ADRDY);
  ADC1->CR |= ADC_CR_ADSTART;
  //Init and Timer3 (19200Hz x 4) for ADC control
  TIM3->ARR = 625 - 1;
  TIM3->CR2 |= TIM_CR2_MMS_1;
  TIM3->CR1 |= TIM_CR1_DIR | TIM_CR1_CEN;
  Buffer_Init();
  //Configure SysTicks every 1ms (48MHz Clock) 
  SysTick_Config(48000);
  NVIC_SetPriority(SysTick_IRQn, 2);
  
  cdc_init_usbd();
  NVIC_EnableIRQ(USB_IRQn);
  NVIC_SetPriority(USB_IRQn, 1);
           
  
      
      
    USB_Enable(true);
    USB_Connect(true);
  
}







// Main loop -----------------------------------------------------------------------------------------------------------
void Loop() {
  bool PowerON  = false;  //Boost converter enabled
//  bool Ready    = false;  //NanoPI ready (false while booting or shutting down)
  bool ExtPower = false;  //External power present
//  bool Charging = false;  //Battery charging
  SysStatus.Bluetooth_ON = 1;
  SysStatus.Bluetooth_BLINK = 1;
  
//  for(uint32_t x = 0; x < 10000000; x++);
  //Suspend();
 
    //Leo entrada alimentacion, carga y nivel bateria (tengo 14 bits)
    
    //LED DE ALIMENTACION
    if((PowerON == false) && (ExtPower == false)) {
      //Apago LED alimentacion
    } else if((PowerON == false) && (ExtPower == true)) {
      //Led en rojo (Si esta cargado fijo, si cargando parpadeo)
    } else if((PowerON == true) && (ExtPower == false)) {
      //Led en amarillo fijo (parpadea si bateria baja)
    } else if((PowerON == true) && (ExtPower == true)) {
      //Led verde (Si esta cargado fijo, si cargando parpadeo)
    }

    //LED DE ESTADO
    //Si el driver no reporta error {
      //Si estoy encendiendo parpadeo verde
      //Si estoy encendido fijo verde
      //Si estoy apagando parpadeo amarillo
      //Si estoy apagado dejo fijo el led en amarillo y al segundo corto alimentacion y apago led
    //} else {
      //El driver decide si es parpadeo o fijo en rojo (00 no error, 01 error leve, 10 error medio, 11 error chungo)
  
    //}
    
    //Leo boton
    //No pulsacion (00)
    //Click es una pulsacion de menos de 1 segundo (01)
    //Long click mas de 5 segundos (10)
    //El boton tambien maneja el buzzer, push down 100ms de beep, a los 5 segundos hago beep beep, a los 15 segundos no hago nada apago de golpe
    //click muy largo 15 segundos (11)
    //click estando apagado -> paso a encendiendo
    //click estando encendido -> envio a driver (toggle BT)
    //click largo estando encendico -> envio a driver (apagar)
    //click muy largo -> apago por webos
    
    //LED WIFI solo muestro estado de lo que me diga el firmware
    //000 apagado
    //001 parpadeo verde
    //010 fijo verde
    //011 parpadeo rojo
    //100 fijo rojo
    //101 parpadeo amarillo
    //110 fijo amarillo
    //111 reserved
    
    //LED BT solo muestro estado de lo que me diga el firmware
    //00 apagao
    //01 parpadeo
    //10 fijo
    //11 reserved

    
}