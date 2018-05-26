/* ================================ openZmeter ================================
 * File:     Main.c
 * Author:   Eduardo Viciana (2017)
 * ---------------------------------------------------------------------------- */
#include "Main.h"
#include "usb_regs.h"
#include "USB_Class.h"
#include "usb_pwr.h"
#include <string.h>
//----------------------------------------------------------------------------------------------------------------------
volatile uint32_t msTicks = 0;
//----------------------------------------------------------------------------------------------------------------------

/* ARM => PC */

void Hardware_Config() {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_ResetBits(GPIOA, GPIO_Pin_8);
  SysTick_Config(SystemCoreClock / 1000);
}
int main(void) {
  Hardware_Config();
  ADC_Configure();
  USB_Configure();

	while(1) {

  
//  ADC_StartConversion(ADC1);
  
  
//  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOS) == RESET) { }
    __NOP();
  }
}

/*******************************************************************************
 * Function Name  : SysTick_Handler
 * Description    : This function handles SysTick Handler.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/

void SysTick_Handler(void) {
  msTicks++;
}
void Waitms(uint32_t ms) {
  uint64_t End = msTicks + ms;
  while(msTicks <= End);
}