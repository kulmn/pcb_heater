/*
 * delay.h
 *
 *  Created on: 18 окт. 2014 г.
 *      Author: yura
 */
//#include "stm32f0xx.h"

#ifndef LIBRARY_DELAY_H_
#define LIBRARY_DELAY_H_

#include <libopencm3/stm32/rcc.h>
//#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
//#include <libopencm3/stm32/memorymap.h>


#define DELAY_TIMER						TIM6
#define DELAY_TIMER_RCC				RCC_TIM6

void delay_init(void);
void delay_us(uint16_t);
void delay_ms(uint16_t);



/*
#define delay_us(delay)											\
{																\
	TIM14->CNT=0;												\
	while (TIM14->CNT < (uint16_t)delay);						\
}


#define delay_ms(delay)											\
{																\
	for(uint16_t i=0; i<(uint16_t)delay; i++) delay_us(1000);	\
}

*/
#endif /* LIBRARY_DELAY_H_ */
