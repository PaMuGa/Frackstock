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

// ch.pascal_mueller.frackstock
static const uint8_t android_package_name[] = {'c', 'h', '.', 'p', 'a', 's', 'c', 'a', 'l', '_',
                                               'm', 'u', 'e', 'l', 'l', 'e', 'r', '.', 'f', 'r',
                                               'a', 'c', 'k', 's', 't', 'o', 'c', 'k'};

uint8_t ndef_msg_buf[256];
												 
												 
/**
 * @brief Callback function for handling NFC events.
 */
void nfc_callback(void * p_context, nfc_t2t_event_t event, const uint8_t * p_data, size_t data_length)
{
    (void)p_context;

    switch (event)
    {
        case NFC_T2T_EVENT_FIELD_ON:
            //LEDS_ON(BSP_LED_0_MASK);
            break;

        case NFC_T2T_EVENT_FIELD_OFF:
            //LEDS_OFF(BSP_LED_0_MASK);
            break;

        default:
            break;
    }
}												 
												 

void nfc_init_app_start(void)
{
	/** @snippet [NFC Launch App usage_1] */
    uint32_t len;
    uint32_t err_code;
    /** @snippet [NFC Launch App usage_1] */

    //NfcRetval ret_val;

    /* Set up NFC */
    err_code = nfc_t2t_setup(nfc_callback, NULL);

    /** @snippet [NFC Launch App usage_2] */
    /*  Provide information about available buffer size to encoding function. */
    len = sizeof(ndef_msg_buf);

    /* Encode launchapp message into buffer */
    err_code = nfc_launchapp_msg_encode(android_package_name,
                                        sizeof(android_package_name),
                                        0,
                                        0,
                                        ndef_msg_buf,
                                        &len);

    //APP_ERROR_CHECK(err_code);
    /** @snippet [NFC Launch App usage_2] */

    /* Set created message as the NFC payload */
    err_code = nfc_t2t_payload_set(ndef_msg_buf, len);

//    if (err_code != NFC_RETVAL_OK)
//    {
//        //APP_ERROR_CHECK((uint32_t) ret_val);
//    }

    /* Start sensing NFC field */
    err_code = nfc_t2t_emulation_start();
//    if (err_code != NFC_RETVAL_OK)
//    {
//        //APP_ERROR_CHECK((uint32_t) ret_val);
//    }
}


void nfc_enter_wakeup_sleep_mode(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	err_code = bsp_nfc_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);
	
	nrf_delay_ms(100);
    // Enter System OFF mode.
    NRF_POWER->SYSTEMOFF = 1;
}

