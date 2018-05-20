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

#include "boardsupport.h"
#include "nrf_gpio.h"

void boardsupport_set_wakeup_gpio(void)
{
	nrf_gpio_cfg_sense_input(PIN_CHG, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
}


uint8_t boardsupport_is_resetreason_nfc(void)
{
	if(NRF_POWER->RESETREAS & POWER_RESETREAS_NFC_Msk)
		return 1;
	else
		return 0;
}
