/*
 * delay.c
 *
 *  Created on: 18 окт. 2014 г.
 *      Author: yura
 */
#include "delay.h"

void delay_init(void)
{
	rcc_periph_clock_enable(DELAY_TIMER_RCC);
	timer_set_prescaler(DELAY_TIMER, (rcc_ahb_frequency/1000000)-1);
	timer_reset(DELAY_TIMER);
	//timer_set_mode(DELAY_TIMER, TIM_CR1_CKD_CK_INT_MUL_2, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_period(DELAY_TIMER, UINT16_MAX);
	timer_enable_counter(DELAY_TIMER);
}

#pragma inline=forced
inline void delay_us(uint16_t delay)
{
	for(uint16_t start_cnt=TIM_CNT(DELAY_TIMER); (uint16_t)(TIM_CNT(DELAY_TIMER)-start_cnt) < delay; );
}


#pragma inline=forced
inline void delay_ms(uint16_t delay)
{
	for(uint16_t i=0;i<delay;i++) delay_us(1000);
}








