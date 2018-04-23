

#include "leds.h"
#include "nfc.h"
#include "lis3de.h"
#include "nrf_delay.h"
#include "stns01.h"
#include "bluetooth.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "SEGGER_RTT.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

void gyro_data_handler(LIS3DE_REGISTER_t, uint8_t);
void ble_data_received_handler(const uint8_t *p_data, uint8_t length);
void ble_adv_timeout_handler(void);

static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

int main()
{
	//uint8_t u8_charge;
	// app started by nfc or gyro?
	
	nfc_init_app_start();
	leds_init();
	stns01_init();
	
	// 
	
	//SEGGER_RTT_WriteString(0, "Startup...\n");
	
	log_init();
	NRF_LOG_INFO("Startup...");
	//NRF_LOG_FLUSH();
	
	bluetooth_init(&ble_data_received_handler, &ble_adv_timeout_handler);
	while(1)
	{
		UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
        power_manage();
		
		//NRF_LOG_PROCESS();
		//NRF_LOG_FLUSH();
		//sd_app_evt_wait();
		//nrf_delay_ms(1);
		//NRF_LOG_INFO("Bla...\n");
		//NRF_LOG_FLUSH();
		
		
	}
	
	//
	//leds_activate();
//	
//	
//	
//	lis3de_init();
//	
//	
//	
	while(1)
	{
//		lis3de_read_async(LIS3DE_REG_STATUS_REG2, &gyro_data_handler);
//		//stns01_get_charge_async(&battery_charge_measured);
//		//u8_charge = stns01_get_charge();
		nrf_delay_ms(100);
	}
	
}

void ble_data_received_handler(const uint8_t *p_data, uint8_t length)
{
	
}

void ble_adv_timeout_handler(void)
{
	nfc_enter_wakeup_sleep_mode();
}


void gyro_data_handler(LIS3DE_REGISTER_t lis3de_register, uint8_t data)
{
	switch(lis3de_register)
	{
		case LIS3DE_REG_STATUS_REG2:
			if(data & 0x8)
			{
				// new data available
			}
			break;
		
		default:
			break;
	}
}


