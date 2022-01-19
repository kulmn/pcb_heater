/*
 * led_7_segm.c
 *
 *  Created on: 26 ���. 2017 �.
 *      Author: kulish_y
 */

#include <indicators/led7seg_driver_spi.h>

/******************************************************************************/
static void led7seg_driver_init(LED7SEG_Interface *interface, uint8_t digits_num)
{
	LED7SEG_SR_Driver *driver = (LED7SEG_SR_Driver*) interface;

	for(uint8_t i = 0; i < digits_num; i++)
		gpio_mode_setup(driver->digit[i].port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, driver->digit[i].pins);

	gpio_mode_setup(driver->spi_cs.port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, driver->spi_cs.pins);
	//HC595_spi_init();
}


/******************************************************************************/
static void led7seg_on_dig(LED7SEG_Interface *interface, uint8_t dig)
{
	LED7SEG_SR_Driver *driver = (LED7SEG_SR_Driver*) interface;
	gpio_set(driver->digit[dig].port, driver->digit[dig].pins);
}


/******************************************************************************
static void led7seg_send(LED7SEG_Interface *interface, uint16_t data)
{
	LED7SEG_SR_Driver *driver = (LED7SEG_SR_Driver*) interface;

	uint16_t		out_data = 0;

	for(uint8_t i = 0; i < 8; i++)
	{
		if (data &  (1 << i) )	out_data |= driver->seg_masks[i];
	}

	HC595_send(out_data);
}

/******************************************************************************/
static void led7seg_send(LED7SEG_Interface *interface, uint16_t data)
{
	LED7SEG_SR_Driver *driver = (LED7SEG_SR_Driver*) interface;

	uint16_t		out_data;
	uint8_t		lo_data = 0, hi_data = 0;

	for(uint8_t i = 0; i < 8; i++)
	{
		if (data &  (1 << i) )	lo_data |= driver->seg_masks[i];
	}

	for(uint8_t i = 0; i < 8; i++)
	{
		if (data &  (1 << (i+8)) )	hi_data |= driver->seg_masks[i];
	}

	out_data = (hi_data << 8) | lo_data;

	gpio_clear(driver->spi_cs.port, driver->spi_cs.pins);
	spi_send(driver->spi, out_data);
	while ((SPI_SR(driver->spi) & SPI_SR_BSY));
	gpio_set(driver->spi_cs.port, driver->spi_cs.pins);
}

/******************************************************************************/
static void led7seg_off_dig(LED7SEG_Interface *interface, uint8_t dig)
{
	LED7SEG_SR_Driver *driver = (LED7SEG_SR_Driver*) interface;
	gpio_clear(driver->digit[dig].port, driver->digit[dig].pins);
}


const LED7SEG_Interface LED7SEG_SR_INTERFACE = { led7seg_driver_init, led7seg_send, led7seg_on_dig, led7seg_off_dig};

