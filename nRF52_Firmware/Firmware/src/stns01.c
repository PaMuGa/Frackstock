#include "stns01.h"

#include "custom_board.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_drv_saadc.h"
#include "nrf_log.h"

void saadc_callback(nrf_drv_saadc_evt_t const * p_event);
uint8_t u8_calculate_charge_percent(uint16_t u16_voltage_x4);
uint16_t stns01_voltage_iir(uint16_t);

//static nrf_saadc_value_t     m_buffer_pool;
//static stns01_charge_measured_handler_t charge_measured_callback;

void stns01_init(void)
{
	// config pin charge_enable and disable charging on default
	//nrf_gpio_cfg_output(PIN_CHG);
	//stns01_enable_disable_charging(0);
	nrf_gpio_cfg(PIN_CHG, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
	
	nrf_gpio_cfg_output(PIN_V_BAT_EN);
	nrf_gpio_pin_clear(PIN_V_BAT_EN);
	
	// init adc
	ret_code_t err_code;
	nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);
	channel_config.gain = NRF_SAADC_GAIN1_6; // div / 6
	channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL; // 0.6V

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);
}

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
	
}

uint8_t stns01_get_charging_state(void)
{
	uint8_t u8_pin = nrf_gpio_pin_read(PIN_CHG);
	if(u8_pin)
		return 0;
	else
		return 1;
}


static uint16_t u16_filtered_voltage_x4;
static nrf_saadc_value_t adc_value[4];
static uint8_t u8_adc_index = 0;

static uint8_t u8_first_measurement = 1;

uint8_t stns01_get_charge(void)
{
	//nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
	nrf_drv_saadc_sample_convert(0, &adc_value[u8_adc_index]);
	u8_adc_index++;
	u8_adc_index %= 4;
	
	if(u8_first_measurement)
	{
		adc_value[1] = adc_value[0];
		adc_value[2] = adc_value[0];
		adc_value[3] = adc_value[0];
		
		u8_first_measurement = 0;
	}
	
	u16_filtered_voltage_x4 = stns01_voltage_iir(adc_value[0] + adc_value[1] + adc_value[2] + adc_value[3]);
	
	return u8_calculate_charge_percent(u16_filtered_voltage_x4);
}

uint16_t stns01_get_battery_voltage(void)
{
	// voltageinput is 12bit, 4096 = 5.2364V at AIN0, 1.278mV per bit
	uint32_t u32_calc_var = u16_filtered_voltage_x4;
	
	u32_calc_var *= 1278;
	u32_calc_var /= 1000;
	
	// now u32_calc_var contains the voltage in mV
	
	return u32_calc_var;
}

uint16_t stns01_voltage_iir(uint16_t u16_new_value)
{
	static uint32_t u32_value = 0;
	static uint8_t u8_first = 0;
	
	if(u8_first == 0)
	{
		u32_value = ((uint32_t)u16_new_value) << 6;
		u8_first = 1;
	}
	
	u32_value = u32_value - (u32_value >> 6) + u16_new_value;
	return u32_value >> 6;
}

uint8_t u8_calculate_charge_percent(uint16_t u16_voltage_x4)
{
	uint16_t u16_battery_mV = stns01_get_battery_voltage();
	
	// simple algorithm, linear: 100% = 4V, 17% 3.5V, 0% = 3V
	
	if(u16_battery_mV >= 4000)
		return 100;
	else if(u16_battery_mV >= 3500)
		return (u16_battery_mV - 3500) / 6 + 17;
	else if(u16_battery_mV >= 3000)
		return (u16_battery_mV - 3752) / 29;
	else
		return 0;
}



