/*
 * This file is part of the PWM-Servo example.
 *
 * Copyright (C) 2011 Stefan Wendler <sw@kaltpost.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PWM_H_
#define __PWM_H_

#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <stdint.h>

/**
 * Initialize the timer used to generate the PWM.
 *
 * @param[in]	rcc_tim				enum rcc_periph_clken . E.g. RCC_TIM16.
 * @param[in]	prescaler			The prescaler value to use. A prescaler devides the system clock speed.
 *							E.g. a deiver of 2 scales a 72Mhz clock down to 36Mhz.
 * @param[in]	period			The period of the PWM. The preiod ends when the timers counter has reached the value given here.
 *							The timers counter is counted up with the prescaled clock freq. With a freq. of 20kHz, the
 *							timer e.g. is increased every 1us.
 */
void pwm_init_timer( uint32_t timer_peripheral, enum rcc_periph_clken rcc_tim, uint32_t prescaler, uint32_t period);

/**
 * Initialize a single ouput chanel for a timer. This function also intializes the GPIOs tight to the timers output channel.
 *
 * @param[in]	timer_peripheral	The timer for which the output channel is to be set. E.g. TIM2.
  * @param[in]	gpio_port			The GPIO port to enable for the timer. E.g. GPIOA.
 * @param[in]	gpio_pin			The GPIO pin to enable for the timer. E.g. GPIO_TIM2_CH2.
  * @param[in]	alt_func				The GPIO alternate func num
 */
void pwm_init_output_channel(uint32_t timer_peripheral, enum tim_oc_id oc_id, uint32_t gpio_port, uint16_t gpio_pin, uint8_t alt_func);

/**
 * Start the timer (and therefroe the PWM). Call this after {@link pwm_init_timer}.
 *
 * @param[in]	timer_peripheral	The timer for which the output channel is to be set. E.g. TIM2.
 */
void pwm_start_timer(uint32_t timer_peripheral);

/**
 * Set the puls width (duty cycle) of the PWMi for a channel. Could by called any time to adjust the period dynamically for a channel.
 *
 * @param[in]	timer_peripheral	The timer for which the output channel is to be set. E.g. TIM2.
 * @param[in]	oc_id			The ID of the timmers output channel. E.g. TIM_OC1.
 * @param[in]	pulse_widht		The pulse with for the channel. Must be between 0 (off) and the value given for period
 * 							with {@link pwm_init_timer}.
 */
void pwm_set_pulse_width(uint32_t timer_peripheral, enum tim_oc_id oc_id, uint32_t pulse_width);

#endif
