/*
 * main.h
 *
 *  Created on: 21 ���. 2014 �.
 *      Author: kulish_y
 */

#ifndef MAIN_H_
#define MAIN_H_


#include <stdint.h>
#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/spi.h>

// FreeRTOS inc
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#include "convert_fn.h"
#include "timer_hal.h"
#include "pwm.h"
#include "pid.h"

#include "indicators/led7seg.h"
#include "indicators/led7seg_driver_sr.h"




typedef enum
{
	DISPL_CLOCK,
	DISPL_DATE,
	DISPL_TEMP_EXT,
	DISPL_TEMP_INT,
} Displ_Params;


#define		DATA_OUT_TASK_FRQ			(500)			//ms
#define		BT_PRESS_DELAY_TIME		( 5 * (1000/DATA_OUT_TASK_FRQ)	)		//s
#define		CLOCK_SHOW_TIME			( 10 * (1000/DATA_OUT_TASK_FRQ) )		//s
#define		PARAM1_SHOW_TIME		(  2 * (1000/DATA_OUT_TASK_FRQ) + CLOCK_SHOW_TIME )
#define		PARAM2_SHOW_TIME		( 2 * (1000/DATA_OUT_TASK_FRQ) +  PARAM1_SHOW_TIME)		//s

/**** PINs defines *******/

// leg7seg
#define LED_IND_DIG_0		GPIOB, GPIO7
#define LED_IND_DIG_1		GPIOB, GPIO6
#define LED_IND_DIG_2		GPIOB, GPIO5


// hc595 masks
#define LED_IND_SEG_A		( (uint8_t) 1 << 5 )
#define LED_IND_SEG_B		( (uint8_t) 1 << 7 )
#define LED_IND_SEG_C		( (uint8_t) 1 << 2 )
#define LED_IND_SEG_D		( (uint8_t) 1 << 3 )
#define LED_IND_SEG_E		( (uint8_t) 1 << 4 )
#define LED_IND_SEG_F		( (uint8_t) 1 << 6 )
#define LED_IND_SEG_G		( (uint8_t) 1 << 1 )
#define LED_IND_SEG_H		( (uint8_t) 1 << 0 )

// buttons
#define BUTTN_HOURS_UP		GPIOB, GPIO0
#define BUTTN_HOURS_DN		GPIOB, GPIO1




#endif /* MAIN_H_ */
