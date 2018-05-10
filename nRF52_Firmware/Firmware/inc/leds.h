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

#ifndef __leds_h
#define __leds_h

#include "nrf.h"

/**
* @brief	LED color structure
*/
typedef struct {
	uint8_t u8_green;
	uint8_t u8_red;
	uint8_t u8_blue;
} led_color_t;

static const led_color_t LED_COLOR_OFF = {0,0,0};
static const led_color_t LED_COLOR_RED = {0,255,0};
static const led_color_t LED_COLOR_GREEN = {255,0,0};
static const led_color_t LED_COLOR_DARK_GREEN = {32,0,0};
static const led_color_t LED_COLOR_BLUE = {0,0,255};
static const led_color_t LED_COLOR_WHITE = {255,255,255};

/**
* @brief	Initializes the LED control Hardware
*/
void leds_init(void);

/**
* @brief	Enables the LED power supply
*/
void leds_activate(void);

/**
* @brief	Disables the LED power supply
*/
void leds_deactivate(void);

/**
* @brief	Updates the led stripe
* @param	led_color_buffer	Array of led_color_t
* @param	u16_length			Array length (stripe length)
*/
void leds_update(led_color_t* led_color_buffer, uint16_t u16_length);

#endif /* __leds_h */
