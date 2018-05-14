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

#include "nrf_drv_i2s.h"

#define NLEDS 60
#define RESET_BITS 32
#define I2S_BUFFER_SIZE 3*NLEDS + RESET_BITS

static uint32_t m_buffer_tx[I2S_BUFFER_SIZE];

// This is the I2S data handler - all data exchange related to the I2S transfers
// is done here.
static void data_handler(uint32_t const * p_data_received,
                         uint32_t       * p_data_to_send,
                         uint16_t         number_of_words)
{
    // Non-NULL value in 'p_data_to_send' indicates that the driver needs
    // a new portion of data to send.
    if (p_data_to_send != NULL)
    {
        // do nothing - buffer is updated elsewhere
    }
}

/*
    caclChannelValue()
    Sets up a 32 bit value for a channel (R/G/B). 
    A channel has 8 x 4-bit codes. Code 0xe is HIGH and 0x8 is LOW.
    So a level of 128 would be represented as:
    0xe8888888
    The 16 bit values need to be swapped because of the way I2S sends data - right/left channels.
    So for the above example, final value sent would be:
    0x8888e888
*/
uint32_t caclChannelValue(uint8_t level)
{
    uint32_t val = 0;

    // 0 
    if(level == 0) {
        val = 0x88888888;
    }
    // 255
    else if (level == 255) {
        val = 0xeeeeeeee;
    }
    else {
        // apply 4-bit 0xe HIGH pattern wherever level bits are 1.
        val = 0x88888888;
        for (uint8_t i = 0; i < 8; i++) {
            if((1 << i) & level) {
                uint32_t mask = ~(0x0f << 4*i);
                uint32_t patt = (0x0e << 4*i);
                val = (val & mask) | patt;
            }
        }

        // swap 16 bits
        val = (val >> 16) | (val << 16);
    }

    return val;
}

void leds_update(led_color_t* led_color_buffer, uint16_t u16_length)
{	
	if(u16_length > NLEDS)
		u16_length = NLEDS;
	
	for(uint16_t i = 0; i < u16_length; i++)
	{
		m_buffer_tx[3*i+0] = caclChannelValue(led_color_buffer[i].u8_green);
		m_buffer_tx[3*i+1] = caclChannelValue(led_color_buffer[i].u8_red);
		m_buffer_tx[3*i+2] = caclChannelValue(led_color_buffer[i].u8_blue);
	}
}

void leds_init(void)
{
	uint32_t err_code;
	
	nrf_gpio_cfg_output(PIN_LED_EN);
	nrf_gpio_pin_clear(PIN_LED_EN);
	
	nrf_gpio_cfg(PIN_PWM1, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_H0H1, NRF_GPIO_PIN_NOSENSE);
	
	nrf_drv_i2s_config_t config = NRF_DRV_I2S_DEFAULT_CONFIG;
    config.sdin_pin  = 0;
	config.sck_pin   = 29;
    config.sdout_pin = PIN_PWM1;
    config.mck_setup = NRF_I2S_MCK_32MDIV10; ///< 32 MHz / 10 = 3.2 MHz.
    config.ratio     = NRF_I2S_RATIO_32X;    ///< LRCK = MCK / 32.
    config.channels  = NRF_I2S_CHANNELS_STEREO;
    
    err_code = nrf_drv_i2s_init(&config, data_handler);
	APP_ERROR_CHECK(err_code);
	
	for(uint16_t i = 0; i < I2S_BUFFER_SIZE; i++)
	{
		m_buffer_tx[i] = 0;
	}
	
	err_code = nrf_drv_i2s_start(0, m_buffer_tx, I2S_BUFFER_SIZE, 0);
	APP_ERROR_CHECK(err_code);
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
