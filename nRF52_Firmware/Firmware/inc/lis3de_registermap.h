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

#include <stdint.h>

#ifndef LIS3DE_REGISTERMAP_H
#define LIS3DE_REGISTERMAP_H

typedef enum {
	LIS3DE_REG_STATUS_REG_AUX	= 0x07,
	LIS3DE_REG_OUT_ADC1_L		= 0x08,
	LIS3DE_REG_OUT_ADC1_H		= 0x09,
	LIS3DE_REG_OUT_ADC2_L		= 0x0A,
	LIS3DE_REG_OUT_ADC2_H		= 0x0B,
	LIS3DE_REG_OUT_ADC3_L		= 0x0C,
	LIS3DE_REG_OUT_ADC3_H		= 0x0D,
	LIS3DE_REG_INT_COUNTER_REG	= 0x0E,
	LIS3DE_REG_WHO_AM_I			= 0x0F,
	LIS3DE_REG_TEMP_CFG_REG		= 0x1F,
	LIS3DE_REG_CTRL_REG1		= 0x20,
	LIS3DE_REG_CTRL_REG2		= 0x21,
	LIS3DE_REG_CTRL_REG3		= 0x22,
	LIS3DE_REG_CTRL_REG4		= 0x23,
	LIS3DE_REG_CTRL_REG5		= 0x24,
	LIS3DE_REG_CTRL_REG6		= 0x25,
	LIS3DE_REG_REFERENCE		= 0x26,
	LIS3DE_REG_STATUS_REG2		= 0x27,
	LIS3DE_REG_OUT_X			= 0x29,
	LIS3DE_REG_OUT_Y			= 0x2B,
	LIS3DE_REG_OUT_Z			= 0x2D,
	LIS3DE_REG_FIFO_CTRL_REG	= 0x2E,
	LIS3DE_REG_FIFO_SRC_REG		= 0x2F,
	LIS3DE_REG_IG1_CFG			= 0x30,
	LIS3DE_REG_IG1_SOURCE		= 0x31,
	LIS3DE_REG_IG1_THS			= 0x32,
	LIS3DE_REG_IG1_DURATION		= 0x33,
	LIS3DE_REG_IG2_CFG			= 0x34,
	LIS3DE_REG_IG2_SOURCE		= 0x35,
	LIS3DE_REG_IG2_DURATION		= 0x37,
	LIS3DE_REG_CLICK_CFG		= 0x38,
	LIS3DE_REG_CLICK_SRC		= 0x39,
	LIS3DE_REG_CLICK_THS		= 0x3A,
	LIS3DE_REG_TIME_LIMIT		= 0x3B,
	LIS3DE_REG_TIME_LATENCY		= 0x3C,
	LIS3DE_REG_TIME_WINDOW		= 0x3D,
	LIS3DE_REG_Act_THS			= 0x3E,
	LIS3DE_REG_Act_DUR			= 0x3F
} LIS3DE_REGISTER_t;

#endif /* LIS3DE_REGISTERMAP_H */
