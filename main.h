/*
 * main.h
 *
 *  Created on: 21 ���. 2014 �.
 *      Author: kulish_y
 */

#ifndef MAIN_H_
#define MAIN_H_


#include <indicators/led7seg_driver_spi.h>
#include <stdint.h>
#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/iwdg.h>
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


#define		MAX_TEMPERATURE			(300)
#define		OVRHT_TEMPERATURE		(50 + MAX_TEMPERATURE)
#define		MIN_TEMPERATURE			(50)

#define		PID_TASK_FRQ				(2000)			//ms
#define		HEATER_OFF_TIME			( 30 *  (60000/PID_TASK_FRQ)	)		//min

#define		DATA_OUT_TASK_FRQ			(100)			//ms
#define		BT_PRESS_DELAY_TIME		( 3 * (1000/DATA_OUT_TASK_FRQ)	)		//s
//#define		CLOCK_SHOW_TIME			( 10 * (1000/DATA_OUT_TASK_FRQ) )		//s
//#define		PARAM1_SHOW_TIME		(  2 * (1000/DATA_OUT_TASK_FRQ) + CLOCK_SHOW_TIME )
//#define		PARAM2_SHOW_TIME		( 2 * (1000/DATA_OUT_TASK_FRQ) +  PARAM1_SHOW_TIME)		//s


#define		DISPL_TEMP_CUR	0
#define		DISPL_TEMP_SET	1

/**** PINs defines *******/
#define SPI1_AF				GPIO_AF0

#define SPI1_SCK			GPIOA, GPIO5
#define SPI1_MISO			GPIOA, GPIO6
#define SPI1_MOSI			GPIOA, GPIO7

#define HC595_CS			GPIOA, GPIO3
#define MAX6675_CS		GPIOA, GPIO4
#ifdef STM32F0
	#define		SPI_GPIO_SPEED		GPIO_OSPEED_HIGH
#else
	#define		SPI_GPIO_SPEED		GPIO_OSPEED_40MHZ
#endif

// Heater pin
#define TIM1_PWM_CH4			GPIOA, GPIO11

// leg7seg
#define LED_IND_DIG_0		GPIOB, GPIO5
#define LED_IND_DIG_1		GPIOB, GPIO6
#define LED_IND_DIG_2		GPIOB, GPIO7


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
typedef enum
{
	BUTTN_UP,
	BUTTN_DN,
	BUTTN_NUM,
} BUTTN;
//#define BUTTN_NUM		2
#define BUTTN_UP_pin		GPIOB, GPIO0
#define BUTTN_DN_pin		GPIOB, GPIO1


/*! \brief P, I and D parameter values
 *
 * The K_P, K_I and K_D values (P, I and D gains)
 * need to be modified to adapt to the application at hand
 */
//! \xrefitem todo "Todo" "Todo list"
#define K_P     2.5
//! \xrefitem todo "Todo" "Todo list"
#define K_I     0.005
//! \xrefitem todo "Todo" "Todo list"
#define K_D     0.00


// Save param address
#define last_page  FLASH_BASE + (15 * 1024)
#define addr_set_temp (last_page)



#endif /* MAIN_H_ */
