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

#define N_LEDS								60		// maximum number of leds
#define SLOW_PROCESS_EXECUTION_TIME			100		// ms
#define EXTREM_SLOW_PROCESS_EXECUTION_TIME	10000 	// ms
#define SLAVE_TIMEOUT_SLEEP					6  		// * EXTREM_SLOW_PROCESS_EXECUTION_TIME seconds

static void log_init(void);
void gyro_data_handler(LIS3DE_REGISTER_t, uint8_t);
void ble_data_received_handler(const uint8_t *p_data, uint8_t length);
void ble_adv_timeout_handler(void);
void ble_connection_handler(uint8_t state);
void acc_xyz_data_handler(lis3de_xyz_acc_data_t acc_data);
void slave_update_handler(uint8_t, uint32_t);
void system_tick_init(void);
void timer_led_event_handler(nrf_timer_event_t event_type, void* p_context);
void update_pattern(void);
void goto_sleep(void);
void nfc_read_handler(void);

// possible application states
typedef enum {
	INITIALIZING,
	ADVERTISING,
	CONNECTED,
	IDLE
} application_state_t;

// application role
typedef enum {
	SLAVE,
	MASTER
} functional_state_t;

application_state_t application_state = INITIALIZING;
functional_state_t functional_state = SLAVE;

const nrf_drv_timer_t TIMER_LED = NRF_DRV_TIMER_INSTANCE(2);	// LED timer instance


uint32_t u32_pattern_control_state = 0x08;	// pattern control state (dark blue)
uint8_t u8_led_length = N_LEDS;			// default led stip length with max n leds
uint8_t u8_selected_pattern = 0;		// default pattern is zero

uint32_t u32_charge;					// actual battery charge in percent
uint8_t u8_charging_enabled;			// 1 if the battery is beeing charged

uint16_t u16_slow_process_counter = SLOW_PROCESS_EXECUTION_TIME - 1;
uint16_t u16_extrem_slow_process_counter = EXTREM_SLOW_PROCESS_EXECUTION_TIME - 1;
uint16_t u16_slave_timeout_timer = SLAVE_TIMEOUT_SLEEP;
uint16_t u16_battery_voltage;

int main()
{
	//nrf_gpio_cfg(21, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_H0H1, NRF_GPIO_PIN_NOSENSE);
	
	// init log
	log_init();
	#ifdef DEBUG
	NRF_LOG_INFO("Startup...");
	NRF_LOG_WARNING("!!DEBUGGING ENABLED!!");
	#endif
	
	// init modules
	//lis3de_init();
	nfc_init_app_start(&nfc_read_handler);
	leds_init();
	stns01_init();
	bluetooth_init(&ble_data_received_handler, &ble_adv_timeout_handler, &ble_connection_handler);
	bluetooth_start_advertising();
	
	application_state = ADVERTISING;
	
	#ifdef DEBUG
	NRF_LOG_INFO("Initialized.");
	#endif
	
	// wait until initialization finished
	nrf_delay_ms(10);
	
	system_tick_init();
	
	slave_scan_init(slave_update_handler);
	functional_state = SLAVE;
	
	#ifdef DEBUG
	NRF_LOG_INFO("Acting as Slave.");
	#endif
	
	//leds_activate();
	
	while(1)
	{
		UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
		
		nrf_delay_ms(1);
		
		u16_slow_process_counter++;
		u16_slow_process_counter %= SLOW_PROCESS_EXECUTION_TIME;
		u16_extrem_slow_process_counter++;
		u16_extrem_slow_process_counter %= EXTREM_SLOW_PROCESS_EXECUTION_TIME;
		
		// measure chare approximatly every tenth second
		if(u16_slow_process_counter == 0)
		{
			u32_charge = stns01_get_charge();
			u16_battery_voltage = stns01_get_battery_voltage();
			
			// send application information to the smartphone if connected
			if(application_state == CONNECTED)
			{
				uint8_t u8_bat_buf[] = {0x01, (uint8_t)u32_charge, (uint8_t)functional_state, u8_selected_pattern,
										u32_pattern_control_state >> 24, (u32_pattern_control_state >> 16) & 0xFF,
										(u32_pattern_control_state >> 8) & 0xFF, u32_pattern_control_state & 0xFF,
										u16_battery_voltage >> 8, u16_battery_voltage & 0xFF};
				bluetooth_send(u8_bat_buf, 10);
			}
		}
		
		// check battery state and enable / disable leds approx. every 10 seconds
		if(u16_extrem_slow_process_counter == 0)
		{
			u8_charging_enabled = stns01_get_charging_state();
			
			if(u8_charging_enabled)
			{
				#ifdef DEBUG
				NRF_LOG_INFO("Charging... Battery: %i, Voltage [mV]: %i", u32_charge, u16_battery_voltage);
				#endif
				patterncontrol_update(CHARGING, u8_led_length, &u32_charge);
				leds_activate();
				nrf_delay_ms(50);
				leds_deactivate();
			} else
			{
				#ifdef DEBUG
				NRF_LOG_INFO("Battery: %i, Voltage [mV]: %i", u32_charge, u16_battery_voltage);
				#endif
				
				// disable LEDs if charge is below 5 percent
				if(u32_charge <= 5)
				{
					#ifdef DEBUG
					NRF_LOG_INFO("Battery low, disable LEDs");
					#endif
					leds_deactivate();
				} else
				{
					leds_activate();
				}
			}
			
			// goto sleep mode if no master is active and the smartphone is disconnected
			if(functional_state == SLAVE && application_state == IDLE && !u8_charging_enabled)
			{
				u16_slave_timeout_timer--;
				
				if(u16_slave_timeout_timer == 0)
				{
					goto_sleep();
				}
			}
		}
	}
}

/**
* @brief Initialize log for RTT Logger
*/
static void log_init(void)
{
	#ifdef DEBUG
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
	#endif
}

void ble_data_received_handler(const uint8_t *p_data, uint8_t length)
{
	if(length < 1) return;
	
	switch(p_data[0]) // control byte
	{
		case 0x0:	// sync time
			u32_pattern_control_state = 0;
		break;
		
		case 0x1: // set led strip length
			u8_led_length = p_data[1];
		break;
		
		case 0x2: // select pattern
			if(functional_state == MASTER)
			{	
				u8_selected_pattern = p_data[1];
				u32_pattern_control_state = 0;
				#ifdef DEBUG
				NRF_LOG_INFO("New pattern: %i", u8_selected_pattern);
				#endif
			}
		break;
		case 0x3: // select master / slave mode
			if(p_data[1] == 1)
			{
				slave_scan_stop();
				functional_state = MASTER;
				master_advertising_init();
				#ifdef DEBUG
				NRF_LOG_INFO("Acting as Master.");
				#endif
			}
			else
			{
				master_advertising_stop();
				functional_state = SLAVE;
				slave_scan_init(slave_update_handler);
				#ifdef DEBUG
				NRF_LOG_INFO("Acting as Slave.");
				#endif
			}
			break;
		case 0x4: // set patterncontrol value (for color mode)
			if(functional_state == MASTER)
			{
				u32_pattern_control_state = p_data[1] << 16 || p_data[2] << 8 || p_data[3];
				u8_selected_pattern = 0;
			}
		break;
	}
}

/**
* @brief Advertising timeout
*/
void ble_adv_timeout_handler(void)
{
	#ifdef DEBUG
	NRF_LOG_INFO("Advertising Timeout.");
	#endif
	application_state = IDLE;
}

/**
* @brief Bluetooth connected / disconnected handler
*/
void ble_connection_handler(uint8_t state)
{
	if(state)
	{
		application_state = CONNECTED;
	}
	else
	{
		// new advertising started by default
		application_state = ADVERTISING;
		
		// switch to slave function if disconnected
		if(functional_state == MASTER)
		{
			master_advertising_stop();
			functional_state = SLAVE;
			slave_scan_init(slave_update_handler);
			#ifdef DEBUG
			NRF_LOG_INFO("Acting as Slave.");
			#endif
		}
	}
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

/**
* @brief LIS3DE new data available handler
*/
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
}

/**
* @brief Interrupt handler for LIS3DE data ready
*/
void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	// start data readout
    lis3de_read_XYZ_async(acc_xyz_data_handler);
}

/**
* @brief Initialize interrupt Handler for LIS3DE
*/
void init_lis3de_data_interrupt(void)
{
	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    nrf_drv_gpiote_in_init(PIN_INT1, &in_config, in_pin_handler);
    nrf_drv_gpiote_in_event_enable(PIN_INT1, true);
}

/**
* @brief Initialize Timer for LED control
*/
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

/**
* @brief Timer timeout handler for updating LEDs
*/
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

/**
* @brief Update LED pattern
*/
void update_pattern(void)
{
	if(u8_selected_pattern != 0) // not for color pattern
		u32_pattern_control_state++;
	
	if(functional_state == MASTER)
	{
		// synchronize with slaves
		update_master_params(&u8_selected_pattern, &u32_pattern_control_state);
	}
	
	if(!u8_charging_enabled)
	{
		patterncontrol_update((pattern_t)(u8_selected_pattern + 2), u8_led_length, &u32_pattern_control_state);
	}
}

/**
* @brief Update control data for slave
*/
void slave_update_handler(uint8_t u8_pattern, uint32_t u32_control_state)
{
	u8_selected_pattern = u8_pattern;
	u32_control_state = u32_control_state;
	u16_slave_timeout_timer = SLAVE_TIMEOUT_SLEEP;
}

/**
* @brief Disables LEDs and enter sleep mode
*/
void goto_sleep(void)
{
	leds_deactivate();
	slave_scan_stop();
	bluetooth_disable();
	
	#ifdef DEBUG
	NRF_LOG_INFO("Going to sleep...");
	#endif
	nrf_delay_ms(200);
	
	nfc_enter_wakeup_sleep_mode();
}

void nfc_read_handler(void)
{
	if(application_state == IDLE)
	{
		#ifdef DEBUG
		NRF_LOG_INFO("Start Advertising by NFC");
		#endif
		bluetooth_start_advertising();
		application_state = ADVERTISING;
	}
}

