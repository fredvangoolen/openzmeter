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
uint64_t Sequence = 0;
uint16_t INBuff[INBUFFER_LEN];
uint16_t INReadPos  = 0;
uint16_t INWritePos = 0;
uint16_t INBuffUsed = 0;
//----------------------------------------------------------------------------------------------------------------------
void ResetBuffers() {
  __disable_irq();
  Sequence   = 0;
  INReadPos  = 0;
  INWritePos = 0;
  INBuffUsed = 0;
  __enable_irq();
}


/* ARM => PC */

void Hardware_Config() {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_ResetBits(GPIOA, GPIO_Pin_8);
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