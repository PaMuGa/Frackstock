/*
* Copyright (c) 2018 Pascal M�ller
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

#include "patterncontrol.h"
#include "leds.h"

void set_pattern_ble_connect(led_color_t*, uint8_t, uint16_t);
void set_pattern_ble_connected(led_color_t*, uint8_t, uint16_t);
void set_pattern_charging(led_color_t*, uint8_t, uint16_t);
void set_pattern_rainbow(led_color_t*,uint8_t,uint16_t*);
void set_pattern_flash(led_color_t*, uint8_t, uint16_t*);
void set_pattern_flashwhite(led_color_t*,uint8_t, uint16_t*);


void reset_pattern(led_color_t* pattern_buffer);

void patterncontrol_update(pattern_t pattern,
	uint8_t u8_pattern_length,
	uint16_t *u16_control_state)
{
	static led_color_t led_color[MAX_N_LEDS];
	static pattern_t last_pattern;
	
	if(u8_pattern_length > MAX_N_LEDS) u8_pattern_length = MAX_N_LEDS;
	
	if(last_pattern != pattern) {
		reset_pattern(led_color);
		last_pattern = pattern;
	}
	
	switch(pattern)
	{
		case BLE_CONNECT:
			set_pattern_ble_connect(led_color, u8_pattern_length, 0);
			break;
		case BLE_CONNECTED:
			set_pattern_ble_connected(led_color, u8_pattern_length, 0);
			break;
		case CHARGING:
			set_pattern_charging(led_color, u8_pattern_length, *u16_control_state);
			break;
		case RAINBOW:     
            set_pattern_rainbow(led_color, u8_pattern_length, u16_control_state);
            break;
        case FLASH:
           set_pattern_flash(led_color, u8_pattern_length, u16_control_state);
        case FLASHWHITE:
            set_pattern_flashwhite(led_color, u8_pattern_length, u16_control_state);     
        case SHIFT:
            for(uint16_t i = 0; i < u8_pattern_length/10; i++)
            {
                led_color[i].u8_red = 0;
                led_color[i].u8_green = 0;
                led_color[i].u8_blue = 0;
			}			 
		case RESET:
		default:	// white
			for(uint16_t i = 0; i < u8_pattern_length; i++)
			{
				led_color[i].u8_red = 0;
				led_color[i].u8_green = 0;
				led_color[i].u8_blue = 0;
			}
			break;
	}
	
	
	leds_update(led_color, u8_pattern_length);
}

void set_pattern_ble_connect(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint16_t u16_control_state)
{
	static uint8_t u8_state = 0;
	
	pattern_buffer[u8_state].u8_blue = 0;
	u8_state++;
	u8_state %= u8_pattern_length;
	pattern_buffer[u8_state].u8_blue = 255;
}


void set_pattern_ble_connected(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint16_t u16_control_state)
{
	for(uint16_t i = 0; i < MAX_N_LEDS; i++)
	{
		pattern_buffer[i].u8_red = 0;
		pattern_buffer[i].u8_green = 0;
		pattern_buffer[i].u8_blue = 255;
	}
}

void reset_pattern(led_color_t* pattern_buffer)
{
	for(uint16_t i = 0; i < MAX_N_LEDS; i++)
	{
		pattern_buffer[i].u8_red = 0;
		pattern_buffer[i].u8_green = 0;
		pattern_buffer[i].u8_blue = 0;
	}
}

void set_pattern_charging(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint16_t u16_control_state)
{
	//static uint8_t u8_state = 0;
	/*static uint8_t charging_timer = 0;
	
	charging_timer++;
	charging_timer %= 500;
	if(charging_timer == 0)
	{*/
	
		if(u16_control_state > 100)
			u16_control_state = 100;
		
		uint16_t u16_max = u8_pattern_length * u16_control_state / 100;
		for(uint8_t i = 0; i < u8_pattern_length; i++)
		{
			if(i < u16_max)
			{
				pattern_buffer[u8_pattern_length - i - 1] = LED_COLOR_DARK_GREEN;
			}
			else
			{
				pattern_buffer[u8_pattern_length - i - 1] = LED_COLOR_OFF;
			}
		}
		
		
	/*}
	else
	{
		reset_pattern(pattern_buffer);
	}*/
}

void set_pattern_rainbow(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint16_t *u16_control_state)
{
	u16_control_state++;
    for(uint16_t i = 0; i < u8_pattern_length/6; i++)
    {
        //rise the color and divide it to hsv space
        pattern_buffer[i].u8_red = *u16_control_state+i;
        pattern_buffer[i].u8_green = 255;
        pattern_buffer[i].u8_blue = 0;
        pattern_buffer[i+u8_pattern_length/4].u8_red = 255;
        pattern_buffer[i+u8_pattern_length/4].u8_green = *u16_control_state%255+i;
        pattern_buffer[i+u8_pattern_length/4].u8_blue = 0;
        pattern_buffer[i+2*u8_pattern_length/4].u8_red = 0;
        pattern_buffer[i+2*u8_pattern_length/4].u8_green = *u16_control_state%255+i;
        pattern_buffer[i+2*u8_pattern_length/4].u8_blue = 255;
        pattern_buffer[i+3*u8_pattern_length/4].u8_red = 0;
        pattern_buffer[i+3*u8_pattern_length/4].u8_green = 255;
        pattern_buffer[i+3*u8_pattern_length/4].u8_blue = *u16_control_state%255+i;
        pattern_buffer[i+4*u8_pattern_length/4].u8_red = 255;
        pattern_buffer[i+4*u8_pattern_length/4].u8_green = *u16_control_state%255+i;
        pattern_buffer[i+4*u8_pattern_length/4].u8_blue = 0;
        pattern_buffer[i+5*u8_pattern_length/4].u8_red = *u16_control_state%255+i;
        pattern_buffer[i+5*u8_pattern_length/4].u8_green = 0;
        pattern_buffer[i+5*u8_pattern_length/4].u8_blue = 255;                     
    }
}

void set_pattern_flash(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint16_t *u16_control_state)
{
	u16_control_state++;
	*u16_control_state %= 100;
    for(uint16_t i = 0; i < u8_pattern_length; i++)
    {
        //flash every 120bpm
        if(*u16_control_state%25)
        {
            pattern_buffer[i].u8_red = 0;
            pattern_buffer[i].u8_green = 0;
            pattern_buffer[i].u8_blue = 0;
        }
        else
        {
            pattern_buffer[i].u8_red = 255-*u16_control_state%255;
            pattern_buffer[i].u8_green = *u16_control_state%255;
            pattern_buffer[i].u8_blue = 255;
        }
    }
}

void set_pattern_flashwhite(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint16_t *u16_control_state)
{
	
    for(uint16_t i = 0; i < u8_pattern_length; i++)
    {
        //flash every 120bpm
        if(*u16_control_state%25)
        {
        pattern_buffer[i].u8_red = 0;
        pattern_buffer[i].u8_green = 0;
        pattern_buffer[i].u8_blue = 0;
        }
        else {

        pattern_buffer[i].u8_red = 255;
        pattern_buffer[i].u8_green = 255;
        pattern_buffer[i].u8_blue = 255;
        }
    }
}


