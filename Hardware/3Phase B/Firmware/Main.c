#include <stdbool.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_gpio.h>
#include "usbd_core.h"
// Local variables -----------------------------------------------------------------------------------------------------
union {
  struct {
    uint8_t Prescaler;        //Increases on each interrupt event
    uint8_t Buzzer;
//    uint8_t Button:6;
    uint8_t Power_A_ON:1;     //Power LED (PB5)
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
    uint8_t ON:1;
    uint8_t READY:1;
  };
  uint32_t Value;
} SysStatus;
struct {
  struct {
    uint16_t VR;
    uint16_t VS;
    uint16_t VT;
    uint16_t IR;
    uint16_t IS;
    uint16_t IT;
    uint16_t IN;    
  } ADC_Sequence[14];
//  uint16_t ReadPos;
//  uint16_t WritePos;
  //uint16_t Buffer[2500];
} ADC_Buffer;
// Control LED, buton and buzzer ---------------------------------------------------------------------------------------
void SysTick_ISR() {
  uint32_t Out = 0;
  if(SysStatus.Prescaler % 2) {
    GPIOA->BSRR = GPIO_BSRR_BS_10;    //A mode (PA10=1)
    GPIOB->BSRR = GPIO_BSRR_BS_5 | GPIO_BSRR_BS_4 | GPIO_BSRR_BS_3;
    if(((SysStatus.Prescaler % 3) == 0) && SysStatus.Power_A_ON  && (!SysStatus.Power_A_BLINK  || (SysStatus.Power_A_BLINK  && (SysStatus.Prescaler < 127)))) Out |= GPIO_BSRR_BR_5;
    if(((SysStatus.Prescaler % 3) == 1) && SysStatus.Status_A_ON && (!SysStatus.Status_A_BLINK || (SysStatus.Status_A_BLINK && (SysStatus.Prescaler < 127)))) Out |= GPIO_BSRR_BR_4;
    if(((SysStatus.Prescaler % 3) == 2) && SysStatus.Wifi_A_ON   && (!SysStatus.Wifi_A_BLINK   || (SysStatus.Wifi_A_BLINK   && (SysStatus.Prescaler < 127)))) Out |= GPIO_BSRR_BR_3;
  } else {
    GPIOA->BSRR = GPIO_BSRR_BR_10;    //A mode (PA10=0)
    GPIOB->BSRR = GPIO_BSRR_BR_5 | GPIO_BSRR_BR_4 | GPIO_BSRR_BR_3;
    if(((SysStatus.Prescaler % 3) == 0) && SysStatus.Power_B_ON  && (!SysStatus.Power_B_BLINK  || (SysStatus.Power_B_BLINK  && (SysStatus.Prescaler < 127)))) Out |= GPIO_BSRR_BS_5;
    if(((SysStatus.Prescaler % 3) == 1) && SysStatus.Status_B_ON && (!SysStatus.Status_B_BLINK || (SysStatus.Status_B_BLINK && (SysStatus.Prescaler < 127)))) Out |= GPIO_BSRR_BS_4;
    if(((SysStatus.Prescaler % 3) == 2) && SysStatus.Wifi_B_ON   && (!SysStatus.Wifi_B_BLINK   || (SysStatus.Wifi_B_BLINK   && (SysStatus.Prescaler < 127)))) Out |= GPIO_BSRR_BS_3;
  }
  GPIOA->BSRR = (((SysStatus.Prescaler % 8) == 0) && SysStatus.Bluetooth_ON && (!SysStatus.Bluetooth_BLINK || (SysStatus.Bluetooth_BLINK && (SysStatus.Prescaler < 127)))) ? GPIO_BSRR_BS_9 : GPIO_BSRR_BR_9;
  GPIOB->BSRR = Out;
  SysStatus.Prescaler++;
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
// Prepare device to run after wakeup or power up ----------------------------------------------------------------------
void InitHardware() {
  //Setup FLASH access time
  FLASH->ACR |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
  //Enable PortA,B,F clock
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOFEN | RCC_AHBENR_DMA1EN;
  //Configure port F (PF0, PF1 -> Digital outputs)
  GPIOF->MODER   |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0;
  GPIOF->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR1;
  //Configure Port A (PA0, PA8 -> Digital inputs, PA1, PA9, PA10, PA15 -> Digital outputs, PA2, PA3, PA4, PA5, PA6, PA7 -> Analog input, PA11, PA12 -> Alternate function, PA13, PA14 -> SWD)
  GPIOA->MODER |= GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7 | GPIO_MODER_MODER9_0 | 
                  GPIO_MODER_MODER10_0 | GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1 | GPIO_MODER_MODER15_0;
  //Configure Port B (PB7 -> Digital input, PB0, PB1 -> Analog inputs, PB3, PB4, PB5, PB6)
  GPIOB->MODER |= GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 | GPIO_MODER_MODER6_0;
  //USB and clock initialization
  RCC->CR2 |= RCC_CR2_HSI48ON;
  RCC->CFGR |= RCC_CFGR_SW_HSI48;
  while((RCC->CR2 & RCC_CR2_HSI48RDY) == 0);
  RCC->APB1ENR |= RCC_APB1ENR_USBEN | RCC_APB1ENR_CRSEN | RCC_APB1ENR_PWREN | RCC_APB1ENR_TIM3EN;
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN | RCC_APB2ENR_ADC1EN;
  //Init Timer3 (19200Hz) for ADC control
  TIM3->ARR = 2500 - 1;
  TIM3->CR2 |= TIM_CR2_MMS_1;
  TIM3->DIER |= TIM_DIER_UDE;
  TIM3->CR1 |= TIM_CR1_CEN;
  //Enable DMA controller
  DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
  DMA1_Channel1->CMAR = (uint32_t)ADC_Buffer.ADC_Sequence;
  DMA1_Channel1->CNDTR = 14;
  DMA1_Channel1->CCR |= DMA_CCR_PL_0 | DMA_CCR_PL_1 | DMA_CCR_PSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_MSIZE_0 | DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_HTIE | DMA_CCR_TCIE | DMA_CCR_EN;

          /*
  NVIC_EnableIRQ(DMA2_Stream4_IRQn);
  NVIC_SetPriority(DMA2_Stream4_IRQn,5);
*/
  
  

  // Initialize the ADC (12MHz, 7.5 adquisition
  ADC1->CFGR2 |= ADC_CFGR2_CKMODE_1;
  ADC1->SMPR |= ADC_SMPR_SMP_0;
  
  
  
  //	ADC1->SQR5 |= ADC_SQR5_SQ1_2; // 1st conversion in regular sequence will be from ADC_IN4
//	ADC1->CR1 &= ~ADC_CR1_RES; // 12-bit resolution (Tconv = 12 ADCCLK cycles)
//	ADC1->CR1 |= ADC_CR1_RES_0; // 10-bit resolution (Tconv = 11 ADCCLK cycles)
//	ADC1->CR1 |= ADC_CR1_RES_1; // 8-bit resolution (Tconv =  9 ADCCLK cycles)
//	ADC1->CR2 &= ~ADC_CR2_ALIGN; // Right alignment
//	ADC1->SMPR3 &= ~ADC_SMPR3_SMP4; // Channel4 sample rate: 4 cycles
//	ADC1->SMPR3 |= ADC_SMPR3_SMP4; // Channel4 sample rate: 384 cycles

  //	ADC1->CR2 |= ADC_CR2_ADON; // Enable the ADC
//	while (!(ADC1->SR & ADC_SR_ADONS)); // Wait until ADC is on
  
 
//  VIC_InitTypeDef NVICInit;

	// Configure the EXTI line 18 connected internally to the USB IP
//	EXTI->PR    =  USB_EXTI_LINE; // Clear IT pending bit for EXTI18
//	EXTI->IMR  |=  USB_EXTI_LINE; // Enable interrupt request from EXTI18
//	EXTI->EMR  &= ~USB_EXTI_LINE; // Disable event on EXTI18
//	EXTI->RTSR |=  USB_EXTI_LINE; // Trigger rising edge enabled
//	EXTI->FTSR &= ~USB_EXTI_LINE; // Trigger falling edge disabled


	

	// Configure USB interrupts
	// 2 bit for pre-emption priority, 2 bits for subpriority
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	// Enable the USB interrupt
//	NVICInit.NVIC_IRQChannel = USB_LP_IRQn;
//	NVICInit.NVIC_IRQChannelPreemptionPriority = 2;
//	NVICInit.NVIC_IRQChannelSubPriority = 0;
//	NVICInit.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVICInit);

	// Enable the USB Wake-up interrupt
//	NVICInit.NVIC_IRQChannel = USB_FS_WKUP_IRQn;
//	NVICInit.NVIC_IRQChannelPreemptionPriority = 1;
//	NVIC_Init(&NVICInit);
    //Configure SysTicks every 2ms (48MHz Clock) 

    SysTick_Config(96000);

  
}




void cdc_init_usbd();
void usbd_enable(usbd_device *dev, bool enable);
extern usbd_device udev;







///////////////////
// STM32L151RBT6 //
///////////////////


// SPL

//#include <stm32l1xx_syscfg.h>
//#include <stm32l1xx_tim.h>
//#include <misc.h>

// USB related stuff
//#include "hw_config.h"
//#include "usb_lib.h"
//#include "usb_desc.h"
//#include "usb_pwr.h"


//#include "pipka.h"
//#include "delay.h"
//#include "uart.h"


//GPIO_InitTypeDef PORT;
//NVIC_InitTypeDef NVICInit;

// ---> usb_endp.c
//extern uint8_t packet_send;
// <--- usb_endp.c


//#define ADC_BUF_SIZE    1024 // ADC buffer size
//#define OVS_COUNT       32   // Oversampling count


//uint8_t ADC_buffer[ADC_BUF_SIZE];

//uint32_t buf_pos;
//uint32_t ADC_buf_pos;
//uint32_t USB_buf_pos;
//uint32_t i;
//uint32_t ovs_accu;
//uint32_t ovs_cntr;




void Main() {
  InitHardware();
  bool PowerON  = false;  //Boost converter enabled
  bool Ready    = false;  //NanoPI ready (false while booting or shutting down)
  bool ExtPower = false;  //External power present
  bool Charging = false;  //Battery charging
  SysStatus.Value = 0xFFFFFFFF;
  
  for(uint32_t x = 0; x < 10000000; x++);
  //Suspend();
  
//  SysTickInfo.LEDs_A = POWER_ON | BT_ON | BT_BLINK;
//  SysTickInfo.LEDs_B = 0xff;
  while(1) {
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
  
  
  
  
  
  









	// Configure TIM9 (trigger for ADC)
//	RCC->APB2ENR |= RCC_APB2Periph_TIM9; // Enable the TIMx peripheral
//	TIM9->ARR   =  SystemCoreClock / (8000 * OVS_COUNT); // Sample rate 8kHz
//	TIM9->CR2  &= ~TIM_CR2_MMS; // Master mode selection reset
//	TIM9->CR2  |=  TIM_CR2_MMS_1; // The update event is selected as trigger output (TRGO)
//	TIM9->DIER |=  TIM_DIER_UIE; // �IMx update interrupt enable
//	TIM9->EGR   =  TIM_EGR_UG; // Generate an update event to reload the prescaler value immediately

	// NVIC interrupt priority group 2
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	// Enable the TIM9 Interrupt
//	NVICInit.NVIC_IRQChannel = TIM9_IRQn;
//	NVICInit.NVIC_IRQChannelPreemptionPriority = 2;
//	NVICInit.NVIC_IRQChannelSubPriority = 0;
//	NVICInit.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVICInit);

/*
	while (1) {
		ADC1->CR2 |= ADC_CR2_SWSTART; // Software ADC start
		while (!(ADC1->SR & ADC_SR_EOC)); // Wait until conversion end
		i = ADC1->DR;
		UART_SendHex16(USART2,i);
		UART_SendStr(USART2," ");
		UART_SendInt(USART2,i);
		UART_SendStr(USART2,"\n");
		Delay_ms(100);
	}

	while(1);
*/

	// Configure USB peripheral
//	USB_HWConfig();

	// Initialize USB device
//	USB_Init();

//	buf_pos = 0;
//	USB_buf_pos = 0;
//	ADC_buf_pos = 0;
//	ovs_accu = 0;
//	ovs_cntr = 0;

//	TIM9->CR1 |= TIM_CR1_CEN; // Enable TIM9
    cdc_init_usbd();
    usbd_enable(&udev, true);
    usbd_connect(&udev, true);
    while(1) {
        usbd_poll(&udev);
    }
    while(1) {
//    	if (packet_send) {
//    		if (_GetENDPOINT(ENDP1) & EP_DTOG_RX) {
//        		UserToPMABufferCopy(&ADC_buffer[USB_buf_pos],ENDP1_BUF0Addr,0x10);
//        		SetEPDblBuf0Count(ENDP1,EP_DBUF_IN,0x10);
//    		} else {
//        		UserToPMABufferCopy(&ADC_buffer[USB_buf_pos],ENDP1_BUF1Addr,0x10);
//        		SetEPDblBuf1Count(ENDP1,EP_DBUF_IN,0x10);
//    		}
//    		FreeUserBuffer(ENDP1,EP_DBUF_IN);
//    		SetEPTxValid(ENDP1);
//    		packet_send = 0;
//
//    		USB_buf_pos += 0x10;
//    		if (USB_buf_pos > ADC_BUF_SIZE - 0x10) USB_buf_pos = 0;
//    	}
    }
}