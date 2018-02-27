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

int main(void) {
  ADC_Configure();
  USB_Configure();

	while(1) {

  
//  ADC_StartConversion(ADC1);
  
  
//  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOS) == RESET) { }
    __NOP();
  }
}