#include "main.h"

SemaphoreHandle_t xSPI_Mutex;
SemaphoreHandle_t xButtons_Smphr[BUTTN_NUM];

const GPIO_HW_PIN button_pins[BUTTN_NUM] = { {BUTTN_UP_pin}, {BUTTN_DN_pin} };



LED7SEG			led_ind;
LED7SEG_SR_Driver	led_driver;
static uint8_t 		led7seg_buf[4];

//! Parameters for regulator
struct PID_DATA pidData;

volatile uint16_t  temp_set_val=0,  temp_cur_val = 0, work_cnt=0;;
uint8_t	shutdn_flag=0, poweroff_flag=0;

uint16_t 			mb_time_in_munutes = 0, loc_time_in_munutes = 0;
uint16_t 			cur_temp=0;



/******************************************************************************/
uint16_t max6675_get_temp(void)
{
	uint16_t spi_data;

	// Clear receive FIFO
	while (SPI_SR(SPI1) & SPI_SR_FRLVL_FIFO_FULL)
		spi_data=spi_read(SPI1);

	gpio_clear(MAX6675_CS);
	spi_data=spi_xfer(SPI1, 0);
	gpio_set(MAX6675_CS);

	if (spi_data & 0x0004)	// Error no sensor
		return 0;
	else
		return  	spi_data= spi_data>>5;
}

/******************************************************************************/
void vPID_Task (void *pvParameters)			//  ~ 20 * 4  bytes of stack used
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = ( PID_TASK_FRQ / portTICK_RATE_MS );
	int16_t pwm_value=0;

	xLastWakeTime = xTaskGetTickCount();
	for( ;; )
    {
		vTaskDelayUntil( &xLastWakeTime, xFrequency );

		xSemaphoreTake(xSPI_Mutex, portMAX_DELAY);
		temp_cur_val = max6675_get_temp();
		xSemaphoreGive(xSPI_Mutex);

		if (work_cnt > HEATER_OFF_TIME)
		{
			work_cnt = HEATER_OFF_TIME+1;
			pwm_value = pid_Controller(0, temp_cur_val, &pidData);
		}
		else
		{
			pwm_value = pid_Controller((int16_t)temp_set_val, temp_cur_val, &pidData);
		}

		if (temp_cur_val == 0) 		// Temperature sensor error
			pwm_set_pulse_width(TIM1, TIM_OC4, 0);
		else
			pwm_set_pulse_width(TIM1, TIM_OC4, pwm_value);


		work_cnt++;

     }
    vTaskDelete(NULL);
}

/******************************************************************************/
void vGetTempTask(void *pvParameters)
{
	portBASE_TYPE xStatus;


	while (1)
	{
		xSemaphoreTake(xSPI_Mutex, portMAX_DELAY);
		temp_cur_val = max6675_get_temp();
//		led7seg_write_uint(&led_ind, temp_cur_val+1);

		xSemaphoreGive(xSPI_Mutex);

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

	for (uint8_t i=0; i<BUTTN_NUM; i++) xButtons_Smphr[i] = xSemaphoreCreateCounting(10,0);

	for( ;; )
	{
		vTaskDelayUntil( &xLastWakeTime, xFrequency );

		for (uint8_t i=0; i<BUTTN_NUM; i++)
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
	const TickType_t xFrequency = (DATA_OUT_TASK_FRQ / portTICK_RATE_MS);		//  ms

	static uint16_t scnt = 0;
	static uint8_t cur_displ  = DISPL_TEMP_CUR;

	xLastWakeTime = xTaskGetTickCount();
	for (;;)
	{
		vTaskDelayUntil(&xLastWakeTime, xFrequency );

		uint8_t button_pressed = 0;
		for (uint8_t i = 0; i < BUTTN_NUM; i++)
		{
			while (xSemaphoreTake( xButtons_Smphr[i], ( portTickType ) 0 ) == pdTRUE)
			{
				scnt = BT_PRESS_DELAY_TIME; 	// Delay after button pressed, sec
				cur_displ = DISPL_TEMP_SET;
				work_cnt=0;						// Reset HeaterOFF counter
				switch (i)
				{
					case BUTTN_UP:	if (temp_set_val < MAX_TEMPERATURE )
						temp_set_val += 5;
					break;
					case BUTTN_DN:	if (temp_set_val>MIN_TEMPERATURE)
						temp_set_val -= 5;
					break;
				}
			}
		}


		switch (cur_displ)
		{
			case DISPL_TEMP_CUR:
				led7seg_write_uint(&led_ind, temp_cur_val);
				break;
			case DISPL_TEMP_SET:
				scnt--;
				if (scnt == 0) cur_displ = DISPL_TEMP_CUR;
				led7seg_write_uint(&led_ind, temp_set_val);
				break;
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
		xSemaphoreTake(xSPI_Mutex, portMAX_DELAY);
		led7seg_update(&led_ind);
		xSemaphoreGive(xSPI_Mutex);
	}
	vTaskDelete(NULL);
}

/******************************************************************************/
void vSaveDataTask (void *pvParameters)
{
	for( ;; )
	{
		uint16_t saved_temp_set_val;

		saved_temp_set_val = (*(volatile uint16_t*) addr_set_temp);

		if (saved_temp_set_val != temp_set_val)
		{
			flash_unlock();
			flash_erase_page(last_page);
			flash_program_half_word(addr_set_temp,temp_set_val);
		}

		vTaskDelay(300000*portTICK_PERIOD_MS);	// 300 sec
	}
	vTaskDelete(NULL);
}

/******************************************************************************/
void spi1_init(void)
{
	gpio_mode_setup(PORT(SPI1_SCK), GPIO_MODE_AF, GPIO_PUPD_NONE, PIN(SPI1_SCK)); // SCK
	gpio_set_output_options(PORT(SPI1_SCK), GPIO_OTYPE_PP, SPI_GPIO_SPEED, PIN(SPI1_SCK));
	gpio_set_af(PORT(SPI1_SCK ), SPI1_AF, PIN(SPI1_SCK ) );

	gpio_mode_setup(PORT(SPI1_MISO), GPIO_MODE_AF, GPIO_PUPD_NONE, PIN(SPI1_MISO));	// MISO
	gpio_set_af(PORT(SPI1_MISO), SPI1_AF, PIN(SPI1_MISO));
	gpio_set_output_options(PORT(SPI1_MISO), GPIO_OTYPE_PP, SPI_GPIO_SPEED, PIN(SPI1_MISO));

	gpio_mode_setup(PORT(SPI1_MOSI), GPIO_MODE_AF, GPIO_PUPD_NONE, PIN(SPI1_MOSI));	// MOSI
	gpio_set_output_options(PORT(SPI1_MOSI), GPIO_OTYPE_PP, SPI_GPIO_SPEED, PIN(SPI1_MOSI));
	gpio_set_af(PORT(SPI1_MOSI ), SPI1_AF, PIN(SPI1_MOSI ) );

	rcc_periph_clock_enable(RCC_SPI1);
	spi_set_master_mode(SPI1);
	spi_set_unidirectional_mode(SPI1);
#ifdef STM32F0
	spi_set_data_size(SPI1,SPI_CR2_DS_16BIT);
#else
	spi_set_dff_16bit(SPI1);
#endif
	spi_set_clock_polarity_0(SPI1);
	spi_set_clock_phase_0(SPI1);
	spi_enable_software_slave_management(SPI1);
	spi_set_nss_high(SPI1);
	spi_set_baudrate_prescaler(SPI1,SPI_CR1_BR_FPCLK_DIV_8);
	spi_send_msb_first(SPI1);
	spi_disable_crc(SPI1);
	spi_enable(SPI1);
}


/******************************************************************************/
void init_led7seg(void)
{
	const GPIO_HW_PIN pins[LED7SEG_DIGITS_NUM] = { { LED_IND_DIG_0 }, { LED_IND_DIG_1 }, { LED_IND_DIG_2 }  };
	const GPIO_HW_PIN hc595_cs = {  HC595_CS  };

	led_ind.num_digits = 3;
	led_ind.cur_digit = 0;
	led_ind.buffer = (uint8_t *)led7seg_buf;
	led_ind.driver = (LED7SEG_Interface*) &led_driver;

	led_driver.interface = LED7SEG_SR_INTERFACE;
	led_driver.spi = SPI1;
	led_driver.spi_cs = hc595_cs;
	for (uint8_t i = 0; i < led_ind.num_digits; i++)
		led_driver.digit[i] = pins[i];
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
void heater_pwm_init(void)
{
	rcc_periph_clock_enable(RCC_TIM1);
	rcc_periph_reset_pulse(RST_TIM1);
	pwm_init_timer( TIM1, (rcc_apb1_frequency/250-1), 500);		// 250Hz  Period=500x4ms=2000 ms
	pwm_init_output_channel(TIM1, TIM_OC4, PORT(TIM1_PWM_CH4), PIN(TIM1_PWM_CH4), GPIO_AF2);
	pwm_set_pulse_width(TIM1, TIM_OC4, 0);
	pwm_start_timer(TIM1);
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

	// 8/2 MHz * 4  = 16MHz
	rcc_set_pll_multiplication_factor(RCC_CFGR_PLLMUL_MUL4);
	rcc_set_pll_source(RCC_CFGR_PLLSRC_HSI_CLK_DIV2);
	rcc_osc_on(RCC_PLL);
	rcc_wait_for_osc_ready(RCC_PLL);
	rcc_set_sysclk_source(RCC_PLL);

	rcc_apb1_frequency = 16000000;
	rcc_ahb_frequency = 16000000;
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
	for (uint8_t i=0; i<BUTTN_NUM; i++)
		gpio_mode_setup(button_pins[i].port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, button_pins[i].pins);


	spi1_init();
	init_led7seg();
	led7seg_write_uint(&led_ind, 245);

	gpio_mode_setup(PORT(MAX6675_CS), GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN(MAX6675_CS));	// CS
	gpio_set(MAX6675_CS);

	temp_set_val = (*(volatile uint16_t*) addr_set_temp);
			if (temp_set_val>MAX_TEMPERATURE) temp_set_val=MIN_TEMPERATURE;
	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData);

	heater_pwm_init();
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
	xSPI_Mutex = xSemaphoreCreateMutex();
//	xTemperQueue=xQueueCreate( 5, sizeof( DS18B20_TypeDef ));
//	xTaskCreate(vGetTempTask,(signed char*)"", configMINIMAL_STACK_SIZE * 2 ,NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(vGetButtonStateTask,(signed char*)"", configMINIMAL_STACK_SIZE * 2,	NULL, tskIDLE_PRIORITY + 1, NULL);

	xTaskCreate(vIndDataOutTask,(signed char*)"", configMINIMAL_STACK_SIZE * 2,	NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(vLed7segUpdateTask,(signed char*)"", configMINIMAL_STACK_SIZE * 1,	NULL, tskIDLE_PRIORITY + 2, NULL);
	xTaskCreate(vPID_Task,(signed char*)"", configMINIMAL_STACK_SIZE * 1,	NULL, tskIDLE_PRIORITY + 1, NULL);

	xTaskCreate(vSaveDataTask,(signed char*)"", configMINIMAL_STACK_SIZE * 1,	NULL, tskIDLE_PRIORITY + 1, NULL);


	vTaskStartScheduler();

	for( ;; );
}
