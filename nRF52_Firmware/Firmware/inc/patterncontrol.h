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

#ifndef __patterncontrol_h
#define __patterncontrol_h

#include "nrf.h"

/// Maximum number of LEDs
#define MAX_N_LEDS	60

/// Available Patterns
typedef enum {
	RESET,
	BLE_CONNECT,
	BLE_CONNECTED,
	CHARGING,
	RAINBOW,
    FLASH,
    FLASHWHITE,
	SHIFT
} pattern_t;

/**
* @brief	Updates the LED pattern
* @param	pattern	Visible pattern
*/
void patterncontrol_update(pattern_t pattern,
	uint8_t u8_pattern_length,
	uint16_t *u16_control_state);

#endif /* __patterncontrol_h */
