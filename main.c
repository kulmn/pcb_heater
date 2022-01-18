#include "main.h"


SemaphoreHandle_t xButtons_Smphr[4];
const GPIO_HW_PIN button_pins[4] = { {BUTTN_HOURS_UP}, {BUTTN_HOURS_DN} };


LED7SEG			led_ind;
LED7SEG_SR_Driver	led_driver;
static uint8_t 		led7seg_buf[4];


uint16_t 			mb_time_in_munutes = 0, loc_time_in_munutes = 0;
uint16_t 			cur_temp=0;


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

		for (uint8_t i=0; i<4; i++)
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
	for (uint8_t i=0; i<4; i++)
		gpio_mode_setup(button_pins[i].port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, button_pins[i].pins);


	init_led7seg();
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
