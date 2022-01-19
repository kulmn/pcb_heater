#include "main.h"


SemaphoreHandle_t xButtons_Smphr[4];
const GPIO_HW_PIN button_pins[4] = { {BUTTN_HOURS_UP}, {BUTTN_HOURS_DN} };


LED7SEG			led_ind;
LED7SEG_SR_Driver	led_driver;
static uint8_t 		led7seg_buf[4];

//! Parameters for regulator
struct PID_DATA pidData;

volatile uint16_t  temp_set_val, fan_set_val, temp_cur_val = 0;
uint8_t	shutdn_flag=0, poweroff_flag=0;

uint16_t 			mb_time_in_munutes = 0, loc_time_in_munutes = 0;
uint16_t 			cur_temp=0;



/******************************************************************************/
uint16_t max6675_get_temp(void)
{
	uint16_t spi_data;

	gpio_clear(SPI1_CS);
	spi_enable(SPI1);
	spi_data=spi_read(SPI1);
	gpio_set(SPI1_CS);
	spi_disable(SPI1);
	if (spi_data & 0x0004)	return 0;
	else return  	spi_data= spi_data>>5;
}

/******************************************************************************/
void vPID_Task(void *pvParameters)	//  ~ 20 * 4  bytes of stack used
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = (300 / portTICK_RATE_MS);	// 200 ms
	int16_t pwm_value = 0;

	uint16_t tempData[16] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t ta = 0;
	uint32_t temp;

	xLastWakeTime = xTaskGetTickCount();
	for (;;)
	{
		vTaskDelayUntil(&xLastWakeTime, xFrequency );

		ta++;
		if (ta == 8) ta = 0;
		tempData[ta] = max6675_get_temp();

		temp = 0;
		for (uint8_t i = 0; i < 8; i++)
			temp += tempData[i];
		temp_cur_val = temp >> 3;

		if (shutdn_flag)
		{
			pwm_set_pulse_width(TIM11, TIM_OC1, 0 );
		} else
		{
			pwm_value = pid_Controller((int16_t) temp_set_val, temp_cur_val, &pidData );

			if (temp_cur_val == 0) pwm_set_pulse_width(TIM11, TIM_OC1, 0 );
			else pwm_set_pulse_width(TIM11, TIM_OC1, pwm_value );
		}

	}
	vTaskDelete(NULL );
}

/******************************************************************************/
void vGetTempTask(void *pvParameters)
{
	portBASE_TYPE xStatus;
	uint16_t temp = 0;
	float t_cur = 0;

	while (1)
	{

		vTaskDelay(1000 );
	}
	vTaskDelete(NULL );
}

/******************************************************************************/
void vGetButtonStateTask (void *pvParameters)			// ~ ???  bytes of stack used
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 10;						// 1000/10ms = 100 Hz
	uint8_t buttons_cnt[] = {0,0,0,0,0,0};

	xLastWakeTime = xTaskGetTickCount();

	for (uint8_t i=0; i<4; i++) xButtons_Smphr[i] = xSemaphoreCreateCounting(10,0);

	for( ;; )
	{
		vTaskDelayUntil( &xLastWakeTime, xFrequency );

		for (uint8_t i=0; i<2; i++)
		{
			if(!gpio_get(button_pins[i].port, button_pins[i].pins))
			{
				buttons_cnt[i]++;
				if( buttons_cnt[i] > 100) {buttons_cnt[i] = 80; xSemaphoreGive( xButtons_Smphr[i] ); }
				if( buttons_cnt[i] == 2) { xSemaphoreGive( xButtons_Smphr[i] ); }
			} else buttons_cnt[i] = 0;
		}

	}
	vTaskDelete(NULL);
}



/******************************************************************************/
void vIndDataOutTask(void *pvParameters)
{
	TickType_t xLastWakeTime;
	portBASE_TYPE xStatus;
	const TickType_t xFrequency = DATA_OUT_TASK_FRQ;		//  ms

	xLastWakeTime = xTaskGetTickCount();
	for (;;)
	{
		vTaskDelayUntil(&xLastWakeTime, xFrequency );

		uint8_t button_pressed = 0;
		for (uint8_t i = 0; i < 4; i++)
		{
			while (xSemaphoreTake( xButtons_Smphr[i], ( portTickType ) 0 ) == pdTRUE)
			{
				button_pressed = 1;
				switch (i)
				{
					case 0:

						break;
					case 1:

						break;
					case 2:

						break;
					case 3:

						break;
				}
			}
		}
		if (button_pressed)
		{

		}




	}

	vTaskDelete(NULL );
}



/******************************************************************************/
void vLed7segUpdateTask(void *pvParameters) 			//  ~ 21 * 4  bytes of stack used
{
	TickType_t xLastWakeTime;
	portBASE_TYPE xStatus;
	const TickType_t xFrequency = 3;		// 333 Hz

	xLastWakeTime = xTaskGetTickCount();
	for( ;; )
	{
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
		led7seg_update(&led_ind);
	}
	vTaskDelete(NULL);
}


/******************************************************************************/
void spi1_init(void)
{
	rcc_periph_clock_enable(RCC_SPI1);
	spi_set_master_mode(SPI1);
	spi_set_unidirectional_mode(SPI1);
#ifdef STM32F0
	spi_set_data_size(SPI1,SPI_CR2_DS_16BIT);
#else
	spi_set_dff_16bit(SPI1);
#endif
	spi_set_clock_polarity_0(SPI1);
	//spi_set_clock_phase_1(SPI1);		// max6675
	spi_set_clock_phase_0(SPI1);		// hc595
	spi_enable_software_slave_management(SPI1);
	spi_set_nss_high(SPI1);
	spi_set_baudrate_prescaler(SPI1,SPI_CR1_BR_FPCLK_DIV_8);
	spi_send_msb_first(SPI1);
	spi_disable_crc(SPI1);
	spi_enable(SPI1);
}

/******************************************************************************/
void max6675_spi_init(void)
{
	gpio_mode_setup(PORT(SPI1_SCK), GPIO_MODE_AF, GPIO_PUPD_NONE, PIN(SPI1_SCK)); // SCK
	gpio_set_af(PORT(SPI1_SCK), GPIO_AF5, PIN(SPI1_SCK));
	gpio_set_output_options(PORT(SPI1_SCK), GPIO_OTYPE_PP, GPIO_OSPEED_40MHZ, PIN(SPI1_SCK));
	gpio_mode_setup(PORT(SPI1_MISO), GPIO_MODE_AF, GPIO_PUPD_NONE, PIN(SPI1_MISO));	// MISO
	gpio_set_af(PORT(SPI1_MISO), GPIO_AF5, PIN(SPI1_MISO));
	gpio_set_output_options(PORT(SPI1_MISO), GPIO_OTYPE_PP, GPIO_OSPEED_40MHZ, PIN(SPI1_MISO));

	gpio_mode_setup(PORT(SPI1_CS), GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN(SPI1_CS));	// CS
	gpio_set(SPI1_CS);

	/* Enable SPI1 Periph and gpio clocks */
	rcc_periph_clock_enable(RCC_SPI1);

//	spi_reset(SPI1);
	spi_set_master_mode(SPI1);
	spi_set_unidirectional_mode(SPI1);
	spi_set_receive_only_mode(SPI1);
#ifdef STM32F0
	spi_set_data_size(SPI1,SPI_CR2_DS_16BIT);
#else
	spi_set_dff_16bit(SPI1);
#endif
	spi_set_clock_polarity_0(SPI1);
	spi_set_clock_phase_1(SPI1);
	spi_enable_software_slave_management(SPI1);
	spi_set_nss_high(SPI1);
	spi_set_baudrate_prescaler(SPI1,SPI_CR1_BR_FPCLK_DIV_16);
	spi_send_msb_first(SPI1);
	spi_disable_crc(SPI1);

}


/******************************************************************************/
void init_led7seg(void)
{
	const GPIO_HW_PIN pins[LED7SEG_DIGITS_NUM] = { { LED_IND_DIG_0 }, { LED_IND_DIG_1 }, { LED_IND_DIG_2 }  };

	led_ind.num_digits = 3;
	led_ind.cur_digit = 0;
	led_ind.buffer = (uint8_t *)led7seg_buf;
	led_ind.driver = (LED7SEG_Interface*) &led_driver;

	for (uint8_t i = 0; i < led_ind.num_digits; i++)
		led_driver.digit[i] = pins[i];

	led_driver.interface = LED7SEG_SR_INTERFACE;
	led_driver.seg_masks[LED7SEG_SEG_A] = LED_IND_SEG_A;
	led_driver.seg_masks[LED7SEG_SEG_B] = (LED_IND_SEG_B);
	led_driver.seg_masks[LED7SEG_SEG_C] = (LED_IND_SEG_C);
	led_driver.seg_masks[LED7SEG_SEG_D] =  (LED_IND_SEG_D);
	led_driver.seg_masks[LED7SEG_SEG_E] =  (LED_IND_SEG_E);
	led_driver.seg_masks[LED7SEG_SEG_F] =  (LED_IND_SEG_F);
	led_driver.seg_masks[LED7SEG_SEG_G] = (LED_IND_SEG_G);
	led_driver.seg_masks[LED7SEG_SEG_H] = (LED_IND_SEG_H);

	led7seg_init(&led_ind);
}


/******************************************************************************/
static void system_clock_setup(void)
{
	// Enable external high-speed oscillator 8MHz.
//	rcc_osc_on(RCC_HSE);
//	rcc_wait_for_osc_ready(RCC_HSE);
//	rcc_set_sysclk_source(RCC_HSE);
	rcc_set_sysclk_source(RCC_HSI);

	rcc_set_hpre(RCC_CFGR_HPRE_NODIV);
	rcc_set_ppre(RCC_CFGR_PPRE_NODIV);

//	flash_set_ws(FLASH_ACR_LATENCY_000_024MHZ);
	flash_set_ws(FLASH_ACR_LATENCY_024_048MHZ);

	// 8/2 MHz * 8  = 32MHz
	rcc_set_pll_multiplication_factor(RCC_CFGR_PLLMUL_MUL8);
	rcc_set_pll_source(RCC_CFGR_PLLSRC_HSI_CLK_DIV2);
	rcc_osc_on(RCC_PLL);
	rcc_wait_for_osc_ready(RCC_PLL);
	rcc_set_sysclk_source(RCC_PLL);

	rcc_apb1_frequency = 32000000;
	rcc_ahb_frequency = 32000000;
}


/******************************************************************************/
//----------------------------
// initializations
//----------------------------
void periphery_init()
{
	system_clock_setup();
	rcc_peripheral_enable_clock(&RCC_APB1ENR ,RCC_APB1ENR_PWREN);				// enable APB1 clocks

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
//	rcc_periph_clock_enable(RCC_GPIOC);
//	rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_GPIOF);

	// Buttons init
	for (uint8_t i=0; i<2; i++)
		gpio_mode_setup(button_pins[i].port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, button_pins[i].pins);


	init_led7seg();
	led7seg_write_uint(&led_ind, 888);

	max6675_spi_init();
	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData);
}


void vApplicationIdleHook( void )
{

}

void vApplicationTickHook( void )
{

}

/******************************************************************************/
int main(void)
{
	periphery_init();
//	xTemperQueue=xQueueCreate( 5, sizeof( DS18B20_TypeDef ));
	xTaskCreate(vGetTempTask,(signed char*)"", configMINIMAL_STACK_SIZE * 2 ,NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(vGetButtonStateTask,(signed char*)"", configMINIMAL_STACK_SIZE * 2,	NULL, tskIDLE_PRIORITY + 1, NULL);

	xTaskCreate(vIndDataOutTask,(signed char*)"", configMINIMAL_STACK_SIZE * 2,	NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(vLed7segUpdateTask,(signed char*)"", configMINIMAL_STACK_SIZE * 1,	NULL, tskIDLE_PRIORITY + 2, NULL);


	vTaskStartScheduler();

	for( ;; );
}
