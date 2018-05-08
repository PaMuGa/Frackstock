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
#include "nrf_drv_timer.h"

#include "custom_board.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "patterncontrol.h"

const nrf_drv_timer_t TIMER_LED = NRF_DRV_TIMER_INSTANCE(2);

void gyro_data_handler(LIS3DE_REGISTER_t, uint8_t);
void ble_data_received_handler(const uint8_t *p_data, uint8_t length);
void ble_adv_timeout_handler(void);
void ble_connection_handler(uint8_t state);
void acc_xyz_data_handler(lis3de_xyz_acc_data_t acc_data);

void system_tick_init(void);
void timer_led_event_handler(nrf_timer_event_t event_type, void* p_context);
void update_pattern(void);

#define N_LEDS	60
led_color_t led_color[N_LEDS];
uint16_t u16_pattern_control_state = 0;
uint8_t u8_led_length = N_LEDS;
uint8_t u8_selected_pattern = 0;

static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

typedef enum {
	INITIALIZING,
	ADVERTISING,
	CONNECTED_IDLE,
	ACTIVE_PATTERN
} application_state_t;

application_state_t application_state = INITIALIZING;

uint16_t cnt = 0;

uint8_t u8_charge;
uint8_t u8_charging_enabled;

uint8_t buf_ble_batt[] = {0x02, 0x00};


#define SLOW_PROCESS_EXECUTION_TIME		100	// ms
uint16_t u16_slow_process_counter = 0;


int main()
{
	
	// app started by nfc or gyro?
	
	// init log
	log_init();
	NRF_LOG_INFO("Startup...");
	
	// init modules
	lis3de_init();
	nfc_init_app_start();
	leds_init();
	stns01_init();
	//bluetooth_init(&ble_data_received_handler, &ble_adv_timeout_handler, &ble_connection_handler);
	
	application_state = ADVERTISING;
	
	NRF_LOG_INFO("Initialized.");
	
	// wait until initialization finished
	nrf_delay_ms(10);
	
	system_tick_init();
	
	leds_activate();
	
	
	while(1)
	{
		UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
		sd_app_evt_wait();
		
		nrf_delay_ms(1);
		
		u16_slow_process_counter++;
		u16_slow_process_counter %= SLOW_PROCESS_EXECUTION_TIME;
		
		if(u16_slow_process_counter == 0)
		{
			u8_charge = stns01_get_charge();
			u8_charging_enabled = stns01_get_charging_state();
		}
	}
}

void ble_data_received_handler(const uint8_t *p_data, uint8_t length)
{
	if(length < 1) return;
	
	switch(p_data[0]) // control byte
	{
		case 0x0:	// sync time
			u16_pattern_control_state = 0;
		break;
		
		case 0x1: // set led strip length
			u8_led_length = p_data[1];
		break;
		
		case 0x2: // select pattern
			application_state = ACTIVE_PATTERN;
			u8_selected_pattern = p_data[1];
		break;
	}
}

void ble_adv_timeout_handler(void)
{
	nfc_enter_wakeup_sleep_mode();
}

void ble_connection_handler(uint8_t state)
{
	if(state)
		application_state = CONNECTED_IDLE;
	else
		application_state = ADVERTISING;
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
	//uint8_t checksum = acc_filtered.acc_x + acc_filtered.acc_y + acc_filtered.acc_z + 0x10;
	
	// SOH | data length | STX | 4 byte data (acc data) | ETX | checksum | EOT
	//uint8_t send_buffer[] = {0x01, 0x04, 0x02, 0x10, acc_filtered.acc_x, acc_filtered.acc_y, acc_filtered.acc_z, 0x03, checksum, 0x04};
	
	//bluetooth_send(send_buffer, sizeof(send_buffer));
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

void system_tick_init(void)
{
	uint32_t time_ms = 20;	// 50 Hz
    uint32_t time_ticks;
    uint32_t err_code = NRF_SUCCESS;

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    err_code = nrf_drv_timer_init(&TIMER_LED, &timer_cfg, timer_led_event_handler);
    APP_ERROR_CHECK(err_code);

    time_ticks = nrf_drv_timer_ms_to_ticks(&TIMER_LED, time_ms);

    nrf_drv_timer_extended_compare(
         &TIMER_LED, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

    nrf_drv_timer_enable(&TIMER_LED);
}

void timer_led_event_handler(nrf_timer_event_t event_type, void* p_context)
{
	switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
            update_pattern();
            break;

        default:
            // Do nothing.
            break;
    }
}

void update_pattern(void)
{
	switch(application_state)
	{
		default:
		case INITIALIZING:
			patterncontrol_update(RESET, u8_led_length, 0);
		break;
		
		case ADVERTISING:
			patterncontrol_update(BLE_CONNECT, u8_led_length, 0);
		break;
		
		case BLE_CONNECTED:
			patterncontrol_update(BLE_CONNECTED, u8_led_length, 0);
		break;
			
		case ACTIVE_PATTERN:
			patterncontrol_update((pattern_t)(u8_selected_pattern + 3), u8_led_length, &u16_pattern_control_state);
		break;
	}
	
}

