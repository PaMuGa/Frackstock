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

#include "nfc.h"

#include "nrf.h"
#include "nrf52_bitfields.h"
#include "nfc_t2t_lib.h"
#include "nfc_launchapp_msg.h"
#include "nrf_error.h"
#include "app_error.h"
#include "nrf_delay.h"

#include "bsp.h"
#include "bsp_nfc.h"

#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"

// ch.pascal_mueller.frackstock
static const uint8_t android_package_name[] = {'c', 'h', '.', 'p', 'a', 's', 'c', 'a', 'l', '_',
                                               'm', 'u', 'e', 'l', 'l', 'e', 'r', '.', 'f', 'r',
                                               'a', 'c', 'k', 's', 't', 'o', 'c', 'k'};

uint8_t ndef_msg_buf[256];

nfc_read_handler_t p_nfc_read_handler;
											   
/**
 * @brief Callback function for handling NFC events.
 */
void nfc_callback(void * p_context, nfc_t2t_event_t event, const uint8_t * p_data, size_t data_length)
{
	if(event == NFC_T2T_EVENT_FIELD_ON)
	{
		if(p_nfc_read_handler != NULL)
		{
			p_nfc_read_handler();
		}
	}
}												 
												 
void nfc_init_app_start(nfc_read_handler_t nfc_read_handler)
{
    uint32_t len;
    uint32_t err_code;
	
	p_nfc_read_handler = nfc_read_handler;
	
	NRF_NFCT->TASKS_DISABLE = 1;
	nrf_delay_ms(1);
	

    /* Set up NFC */
    err_code = nfc_t2t_setup(nfc_callback, NULL);

    len = sizeof(ndef_msg_buf);

    /* Encode launchapp message into buffer */
    err_code = nfc_launchapp_msg_encode(android_package_name,
                                        sizeof(android_package_name),
                                        0,
                                        0,
                                        ndef_msg_buf,
                                        &len);
    APP_ERROR_CHECK(err_code);

    /* Set created message as the NFC payload */
    err_code = nfc_t2t_payload_set(ndef_msg_buf, len);
	APP_ERROR_CHECK(err_code);						
										
    err_code = nfc_t2t_emulation_start();
	APP_ERROR_CHECK(err_code);							
}


ret_code_t err_code = NRF_SUCCESS;
void nfc_enter_wakeup_sleep_mode(void)
{
	err_code = nfc_t2t_emulation_stop();;
	//err_code = bsp_nfc_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);
	
	NRF_NFCT->TASKS_SENSE = 1;
	
	nrf_delay_ms(50);
	
    // Enter System OFF mode.
    NRF_POWER->SYSTEMOFF = 1;
	
	
}

