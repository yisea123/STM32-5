#ifndef __BSP_SYSTICK_H
#define __BSP_SYSTICK_H

#include "stm32f10x.h"

//extern __IO uint32_t TimingDelay; 

void SysTick_Init(void);
void delay_us(__IO uint32_t nTime);
void delay_ms(__IO uint32_t nTime);

#endif
