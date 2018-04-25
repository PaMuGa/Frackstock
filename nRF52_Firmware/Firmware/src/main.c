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
#include "nrf_drv_gpiote.h"

#include "custom_board.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

void gyro_data_handler(LIS3DE_REGISTER_t, uint8_t);
void ble_data_received_handler(const uint8_t *p_data, uint8_t length);
void ble_adv_timeout_handler(void);
void acc_xyz_data_handler(lis3de_xyz_acc_data_t acc_data);

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
	
	// init log
	log_init();
	NRF_LOG_INFO("Startup...");
	
	// init modules
	lis3de_init();
	nfc_init_app_start();
	leds_init();
	stns01_init();
	bluetooth_init(&ble_data_received_handler, &ble_adv_timeout_handler);
	
	// wait until initialization finished
	nrf_delay_ms(10);
	
	
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
//	while(1)
//	{
//		lis3de_read_async(LIS3DE_REG_STATUS_REG2, &gyro_data_handler);
//		//stns01_get_charge_async(&battery_charge_measured);
//		//u8_charge = stns01_get_charge();
//		nrf_delay_ms(100);
//	}
	
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

#define ACC_DATA_FIR_FILTER_LENGTH	4	// fs = 100Hz

static lis3de_xyz_acc_data_t lis3de_xyz_acc_data_fir_buffer[ACC_DATA_FIR_FILTER_LENGTH - 1];
static uint8_t u8_filter_buffer_index_counter = 0;

void acc_xyz_data_handler(lis3de_xyz_acc_data_t acc_data)
{
	// filter data
	lis3de_xyz_acc_data_t acc_filtered = acc_data;
	
	for(uint8_t i = 0; i < ACC_DATA_FIR_FILTER_LENGTH - 1; i++)
	{
		acc_filtered.acc_x += lis3de_xyz_acc_data_fir_buffer[i].acc_x;
		acc_filtered.acc_y += lis3de_xyz_acc_data_fir_buffer[i].acc_y;
		acc_filtered.acc_z += lis3de_xyz_acc_data_fir_buffer[i].acc_z;
	}
	
	lis3de_xyz_acc_data_fir_buffer[u8_filter_buffer_index_counter] = acc_data;
	u8_filter_buffer_index_counter++;
	u8_filter_buffer_index_counter %= ACC_DATA_FIR_FILTER_LENGTH - 1;
	
	acc_filtered.acc_x /= ACC_DATA_FIR_FILTER_LENGTH;
	acc_filtered.acc_y /= ACC_DATA_FIR_FILTER_LENGTH;
	acc_filtered.acc_z /= ACC_DATA_FIR_FILTER_LENGTH;
	
	// send data over ble
	
	uint8_t checksum = acc_filtered.acc_x + acc_filtered.acc_y + acc_filtered.acc_z + 0x10;
	
	// SOH | data length | STX | 4 byte data (acc data) | ETX | checksum | EOT
	uint8_t send_buffer[] = {0x01, 0x04, 0x02, 0x10, acc_filtered.acc_x, acc_filtered.acc_y, acc_filtered.acc_z, 0x03, checksum, 0x04};
	
	bluetooth_send(send_buffer, sizeof(send_buffer));
}

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	// start data readout
    lis3de_read_XYZ_async(acc_xyz_data_handler);
}

void init_lis3de_data_interrupt(void)
{
	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    nrf_drv_gpiote_in_init(PIN_INT1, &in_config, in_pin_handler);
    nrf_drv_gpiote_in_event_enable(PIN_INT1, true);
}

