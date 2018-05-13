/*
* Copyright (c) 2018 Pascal Müller
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
* USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "leds.h"
#include "custom_board.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "system_nrf52.h"
#include "app_util_platform.h"

void leds_update(led_color_t* led_color_buffer, uint16_t u16_length)
{	
	uint8_t *u8_color_buffer_byte = (uint8_t*)led_color_buffer;
	
	uint16_t u16_nbits = u16_length * 3 * 8;
	
	//__disable_irq();
	CRITICAL_REGION_ENTER();
	
	for(uint16_t i = 0; i < u16_nbits; i++)
	{
		uint8_t u8_byte = u8_color_buffer_byte[i >> 3];
		uint8_t u8_mask = 1 << (7-i % 8);

		if(!(u8_byte & u8_mask)) // '0'
		{
			nrf_gpio_pin_set(PIN_PWM1);
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			nrf_gpio_pin_clear(PIN_PWM1);
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			
			// for L3 optimizations
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
		}
		else	// '1'
		{
			nrf_gpio_pin_set(PIN_PWM1);
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			
			// for L3 optimizations
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			nrf_gpio_pin_clear(PIN_PWM1);
			
			// for L3 optimizations
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
			__NOP();
		}
	}
	
	nrf_gpio_pin_clear(PIN_PWM1);
	
	//__enable_irq();
	CRITICAL_REGION_EXIT();
	
	nrf_delay_us(50);	// wait at least 50us
}

void leds_init(void)
{
	nrf_gpio_cfg(PIN_PWM1, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_H0H1, NRF_GPIO_PIN_NOSENSE);
	
	nrf_gpio_cfg_output(PIN_LED_EN);
	nrf_gpio_pin_clear(PIN_LED_EN);
}


void leds_activate(void)
{
	// Enable power supply
	nrf_gpio_pin_set(PIN_LED_EN);
	
	// wait until power ok
	nrf_delay_ms(1);
}

void leds_deactivate(void)
{
	// disable power supply
	nrf_gpio_pin_clear(PIN_LED_EN);
}
