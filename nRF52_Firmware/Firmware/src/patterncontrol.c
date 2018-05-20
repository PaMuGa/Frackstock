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

#include "patterncontrol.h"
#include "leds.h"

void set_pattern_ble_connect(led_color_t*, uint8_t, uint32_t);
void set_pattern_ble_connected(led_color_t*, uint8_t, uint32_t);
void set_pattern_charging(led_color_t*, uint8_t, uint32_t*);
void set_pattern_color(led_color_t*, uint8_t, uint32_t*);
//void set_pattern_rainbow(led_color_t*,uint8_t,uint32_t*);
void set_pattern_flash(led_color_t*, uint8_t, uint32_t*);
void set_pattern_flashwhite(led_color_t*,uint8_t, uint32_t*);
void set_pattern_purple_rain(led_color_t*, uint8_t, uint32_t *);
void set_pattern_shift(led_color_t*, uint8_t, uint32_t *);
void set_pattern_colorfull(led_color_t*, uint8_t, uint32_t *);
void HSVtoRGB(uint16_t h, uint16_t s, uint16_t v, uint16_t dest[3]);
void reset_pattern(led_color_t* pattern_buffer);

#define BRIGHTNESS 40


void patterncontrol_update(pattern_t pattern,
	uint8_t u8_pattern_length,
	uint32_t *u32_control_state)
{
	static led_color_t led_color[MAX_N_LEDS];
	
	if(u8_pattern_length > MAX_N_LEDS) u8_pattern_length = MAX_N_LEDS;
	
	switch(pattern)
	{
		case CHARGING:
			set_pattern_charging(led_color, u8_pattern_length, u32_control_state);
			break;
		case COLOR:
			set_pattern_color(led_color, u8_pattern_length, u32_control_state);
			break;
//		case RAINBOW:     
//            set_pattern_rainbow(led_color, u8_pattern_length, u32_control_state);
//            break;
        case FLASH:
           set_pattern_flash(led_color, u8_pattern_length, u32_control_state);
			break;
        case FLASHWHITE:
            set_pattern_flashwhite(led_color, u8_pattern_length, u32_control_state);
			break;
		case PURPLE_RAIN:
			set_pattern_purple_rain(led_color, u8_pattern_length, u32_control_state);
			break;
        case SHIFT:
            set_pattern_shift(led_color, u8_pattern_length, u32_control_state);
			break;
		case COLORFULL:
			set_pattern_colorfull(led_color, u8_pattern_length, u32_control_state);
			break;
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
	uint32_t u32_control_state)
{
	static uint8_t u8_state = 0;
	
	pattern_buffer[u8_state].u8_blue = 0;
	u8_state++;
	u8_state %= u8_pattern_length;
	pattern_buffer[u8_state].u8_blue = 255;
}

void set_pattern_purple_rain(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint32_t *u32_control_state)
{
	static uint32_t u32_last_index = 0;

	pattern_buffer[u32_last_index >> 2] = LED_COLOR_OFF;
	
	*u32_control_state %= 240;
	u32_last_index = *u32_control_state;
	
	pattern_buffer[*u32_control_state >> 2].u8_blue = 255;
	pattern_buffer[*u32_control_state >> 2].u8_red = 255;
}


void set_pattern_ble_connected(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint32_t u32_control_state)
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
	uint32_t *u32_control_state)
{
	if(*u32_control_state > 100)
		*u32_control_state = 100;
	
	uint16_t u16_max = u8_pattern_length * *u32_control_state / 100;
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
}

void set_pattern_color(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint32_t *u32_control_state)
{
	led_color_t configured_color;
	configured_color.u8_red = *u32_control_state >> 16;
	configured_color.u8_green = (*u32_control_state >> 8) & 0xFF;
	configured_color.u8_blue = *u32_control_state & 0xFF;
	
    for(uint16_t i = 0; i < u8_pattern_length; i++)
    {
		pattern_buffer[i] = configured_color;
    }
}

void set_pattern_rainbow(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint32_t *u32_control_state)
{
    static uint16_t rgb[3];
    static float circle;
	
    for(uint16_t i = 0; i < MAX_N_LEDS; i++)
    {
        //scale circle with counter between 0 to 1
        circle= i*(float)(360/MAX_N_LEDS)*(float)*u32_control_state/1000;
        HSVtoRGB((uint16_t)circle,80,40, rgb);
        
        pattern_buffer[i].u8_red = rgb[0];
        pattern_buffer[i].u8_green = rgb[1];
        pattern_buffer[i].u8_blue = rgb[2];
    }
    //reset counter
    if(*u32_control_state>=1000) {
        *u32_control_state=0;
    }
}

void set_pattern_flash(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint32_t *u32_control_state)
{
    for(uint16_t i = 0; i < u8_pattern_length; i++)
    {
        //flash every 120bpm
        if(*u32_control_state<140)
        {
            pattern_buffer[i] = LED_COLOR_OFF;
        }
        else
        {
            pattern_buffer[i].u8_red = 20-i;
            pattern_buffer[i].u8_green = i+20;
            pattern_buffer[i].u8_blue = BRIGHTNESS;
        }
    }
    //reset counter
    if(*u32_control_state>140) {
        *u32_control_state=0;
    }
}


void set_pattern_flashwhite(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint32_t *u32_control_state)
{
 for(uint16_t i = 0; i < u8_pattern_length; i++)
    {
        //flash every 120bpm
        if(*u32_control_state<140)
        {
            pattern_buffer[i] = LED_COLOR_OFF;
        }
        else
        {
            pattern_buffer[i].u8_red = BRIGHTNESS;
            pattern_buffer[i].u8_green = BRIGHTNESS;
            pattern_buffer[i].u8_blue = BRIGHTNESS;
        }
    }
    //reset counter
    if(*u32_control_state>140) {
        *u32_control_state=0;
    }
}

void set_pattern_shift(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint32_t *u32_control_state)
{
    static const uint8_t shift_length=10;
    static uint16_t random_color[3];

    
     for(uint16_t i = 0; i < u8_pattern_length; i++)
            {
                pattern_buffer[i].u8_red   = 0;
                pattern_buffer[i].u8_green = 0;
                pattern_buffer[i].u8_blue  = 0;
			}
       // set pattern to one
    for(uint16_t i = 0; i < shift_length; i++)
    {
        //end of led pattern
        pattern_buffer[u8_pattern_length-shift_length-*u32_control_state+i].u8_red    = 10;
        pattern_buffer[u8_pattern_length-shift_length-*u32_control_state+i].u8_green  = BRIGHTNESS/2;
        pattern_buffer[u8_pattern_length-shift_length-*u32_control_state+i].u8_blue   = BRIGHTNESS;
        //begin of led Pattern
        pattern_buffer[*u32_control_state+i].u8_red    = 10;
        pattern_buffer[*u32_control_state+i].u8_green  = BRIGHTNESS/2;
        pattern_buffer[*u32_control_state+i].u8_blue   = BRIGHTNESS;
    }          
    //reset counter
    if(*u32_control_state>=u8_pattern_length) {
        *u32_control_state=0;
            /*get random color from modulo counter
    @ TODO implement a real random counter*/
    //HSVtoRGB((*u32_control_state*2)%359,80,BRIGHTNESS, random_color);
    }
}

void set_pattern_colorfull(led_color_t* pattern_buffer,
	uint8_t u8_pattern_length,
	uint32_t *u32_control_state)
{
    //
    static uint16_t rgb[3];
    //convert counter to rgb
    HSVtoRGB(*u32_control_state/4,80,20, rgb);

     for(uint16_t i = 0; i < u8_pattern_length; i++)
            {

                pattern_buffer[i].u8_red = rgb[0];
                pattern_buffer[i].u8_green = rgb[1];
                pattern_buffer[i].u8_blue = rgb[2];
			}
    //reset counter
    if(*u32_control_state>1440) {
        *u32_control_state=0;
    }
}

int32_t abs(int32_t x)
{
	if(x < 0)
		return -x;
	else
		return x;
}

/** ***************************************************************************
 * @brief conversion HSV to RGB with fixed point scale 100^2 conversion
 ** @param [in] h [0..360]
 ** @param [in] s [0.100]
 ** @param [in] v [0.100]
 ** @param [out] destination array
 *****************************************************************************/
void HSVtoRGB(uint16_t h, uint16_t s, uint16_t v, uint16_t dest[3])
{
	uint16_t scale = 100;
	uint16_t c, x, m;
	uint16_t rgb_s[3];
	uint16_t hue_scale= 1000;

	c= v*s;// Chroma Value
	//double hue_f= fmod((double)h/60,2);

	x= c*(hue_scale-abs(h*hue_scale/60%(2*hue_scale) - hue_scale))/hue_scale; 	//
	m= v*(scale-s); 	// proportion of white

	switch(h/60){
	case 0:
			rgb_s[0]= c;
			rgb_s[1]= x;
			rgb_s[2]= 0;
			break;
	case 1:
			rgb_s[0]= x;
			rgb_s[1]= c;
			rgb_s[2]= 0;
			break;
	case 2:
			rgb_s[0]= 0;
			rgb_s[1]= c;
			rgb_s[2]= x;
			break;
	case 3:
			rgb_s[0]= 0;
			rgb_s[1]= x;
			rgb_s[2]= c;
			break;
	case 4:
			rgb_s[0]= x;
			rgb_s[1]= 0;
			rgb_s[2]= c;
			break;
	case 5:
			rgb_s[0]= c;
			rgb_s[1]= 0;
			rgb_s[2]= x;
			break;

	default:
			break;
		}

	dest[0]= (rgb_s[0]+m)*255/(scale*scale);
	dest[1]= (rgb_s[1]+m)*255/(scale*scale);
	dest[2]= (rgb_s[2]+m)*255/(scale*scale);
}



