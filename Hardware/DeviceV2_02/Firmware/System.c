void Setup();
void Loop();
void SysTick_ISR();
void DMA_CH1_ISR();
void USB_ISR();
static void HardFault_Handler( void ) __attribute__( ( naked ) );
//----------------------------------------------------------------------------------------------------------------------
void Init(void);
void Default_Handler(void);
// The following are 'declared' in the linker script -------------------------------------------------------------------
extern unsigned char  INIT_DATA_VALUES;
extern unsigned char  INIT_DATA_START;
extern unsigned char  INIT_DATA_END;
extern unsigned char  BSS_START;
extern unsigned char  BSS_END;
// the section "vectors" is placed at the beginning of flash  by the linker script -------------------------------------
const void * Vectors[] __attribute__((section(".vectors"))) = {
  (void *)0x20001800,     /* Top of stack */
  Init,                   /* Reset Handler */
  Default_Handler,        /* NMI */
  Default_Handler,        /* Hard Fault */
  Default_Handler,        /* MemManage */
  Default_Handler,        /* Reserved  */
  Default_Handler,        /* Reserved */
  Default_Handler,        /* Reserved */
  Default_Handler,        /* Reserved */
  Default_Handler,        /* Reserved */
  Default_Handler,        /* Reserved */
  Default_Handler,        /* SVC Call */
  Default_Handler,        /* Reserved */
  Default_Handler,        /* Reserved */
  Default_Handler,        /* PendSV */
  SysTick_ISR,            /* SysTick */
  // External interrupt handlers follow
  Default_Handler,        /* 0: WWDG */
  Default_Handler,        /* 1: PVD and VDDIO2 supply comparator interrupt */
  Default_Handler,        /* 2: RTC */
  Default_Handler,        /* 3: FLASH */
  Default_Handler,        /* 4: RCC */
  Default_Handler,        /* 5: EXTI0_1 */
  Default_Handler,        /* 6: EXTI2_3 */
  Default_Handler,        /* 7: EXTI4_15 */
  Default_Handler,        /* 8: TSC */
  DMA_CH1_ISR,            /* 9: DMA_CH1 */
  Default_Handler,        /* 10: DMA_CH2_3, DMA2_CH_1_2 */
  Default_Handler,        /* 11: DMA_CH4_5_6_7, DMA2_CH3_4_5 */
  Default_Handler,        /* 12: ADC_COMP */
  Default_Handler,        /* 13: TIM1_BRK_UP_TRG_COM */
  Default_Handler,        /* 14: TIM1_CC */
  Default_Handler,        /* 15: TIM2 */
  Default_Handler,        /* 16: TIM3 */
  Default_Handler,        /* 17: TIM6_DAC*/
  Default_Handler,        /* 18: TIM7 */
  Default_Handler,        /* 19: TIM14 */
  Default_Handler,        /* 20: TIM15 */
  Default_Handler,        /* 21: TIM16 */
  Default_Handler,        /* 22: TIM17 */
  Default_Handler,        /* 23: I2C1 */
  Default_Handler,        /* 24: I2C2 */
  Default_Handler,        /* 25: SPI1 */
  Default_Handler,        /* 26: SPI2 */
  Default_Handler,        /* 27: USART1 */
  Default_Handler,        /* 28: USART2 */
  Default_Handler,        /* 29: USART3_4_5_6_7_8 */
  Default_Handler,        /* 30: CEC_CAN */
  USB_ISR                 /* 31: USB */
};
void Init() {
  // Init data section
  unsigned char *src  = &INIT_DATA_VALUES;
  unsigned char *dest = &INIT_DATA_START;
  unsigned len = &INIT_DATA_END - &INIT_DATA_START;
  while(len--) *dest++ = *src++;
  // Zero bss section
  dest = &BSS_START;
  len = &BSS_END - &BSS_START;
  while(len--) *dest++=0;
  Setup();
  while(1) Loop();
}
void Default_Handler() {
  while(1);
}
/*void HardFault_HandlerC(unsigned long *hardfault_args){
  volatile unsigned long stacked_r0 ;
  volatile unsigned long stacked_r1 ;
  volatile unsigned long stacked_r2 ;
  volatile unsigned long stacked_r3 ;
  volatile unsigned long stacked_r12 ;
  volatile unsigned long stacked_lr ;
  volatile unsigned long stacked_pc ;
  volatile unsigned long stacked_psr ;
  volatile unsigned long _CFSR ;
  volatile unsigned long _HFSR ;
  volatile unsigned long _DFSR ;
  volatile unsigned long _AFSR ;
  volatile unsigned long _BFAR ;
  volatile unsigned long _MMAR ;
  stacked_r0 = ((unsigned long)hardfault_args[0]) ;
  stacked_r1 = ((unsigned long)hardfault_args[1]) ;
  stacked_r2 = ((unsigned long)hardfault_args[2]) ;
  stacked_r3 = ((unsigned long)hardfault_args[3]) ;
  stacked_r12 = ((unsigned long)hardfault_args[4]) ;
  stacked_lr = ((unsigned long)hardfault_args[5]) ;
  stacked_pc = ((unsigned long)hardfault_args[6]) ;
  stacked_psr = ((unsigned long)hardfault_args[7]) ;
  // Configurable Fault Status Register
  // Consists of MMSR, BFSR and UFSR
  _CFSR = (*((volatile unsigned long *)(0xE000ED28))) ;
  // Hard Fault Status Register
  _HFSR = (*((volatile unsigned long *)(0xE000ED2C))) ;
  // Debug Fault Status Register
  _DFSR = (*((volatile unsigned long *)(0xE000ED30))) ;
  // Auxiliary Fault Status Register
  _AFSR = (*((volatile unsigned long *)(0xE000ED3C))) ;
  // Read the Fault Address Registers. These may not contain valid values.
  // Check BFARVALID/MMARVALID to see if they are valid values
  // MemManage Fault Address Register
  _MMAR = (*((volatile unsigned long *)(0xE000ED34))) ;
  // Bus Fault Address Register
  _BFAR = (*((volatile unsigned long *)(0xE000ED38))) ;
  __asm("BKPT #0\n") ; // Break into the debugger
}
static void HardFault_Handler(void) {
 __asm(  ".syntax unified\n"
         "MOVS   R0, #4  \n"
         "MOV    R1, LR  \n"
         "TST    R0, R1  \n"
         "BEQ    _MSP    \n"
         "MRS    R0, PSP \n"
         "B      HardFault_HandlerC      \n"
         "_MSP:  \n"
         "MRS    R0, MSP \n"
         "B      HardFault_HandlerC      \n"
         ".syntax divided\n");
}*/