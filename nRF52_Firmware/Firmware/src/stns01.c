#include "stns01.h"

#include "custom_board.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_drv_saadc.h"

void saadc_callback(nrf_drv_saadc_evt_t const * p_event);
uint8_t u8_calculate_charge_percent(int16_t i16_voltage_x4);

//static nrf_saadc_value_t     m_buffer_pool;
//static stns01_charge_measured_handler_t charge_measured_callback;

void stns01_init(void)
{
	// config pin charge_enable and disable charging on default
	nrf_gpio_cfg_output(PIN_CHG);
	stns01_enable_disable_charging(0);
	
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

void stns01_enable_disable_charging(uint8_t u8_enalbe)
{
	if(u8_enalbe)
		nrf_gpio_pin_set(PIN_CHG);
	else
		nrf_gpio_pin_clear(PIN_CHG);
}


uint8_t stns01_get_charge(void)
{
	//charge_measured_callback = callback;
	nrf_saadc_value_t adc_value;
	
	//nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
	nrf_drv_saadc_sample_convert(0, &adc_value);
	
	return u8_calculate_charge_percent(adc_value << 2);
}

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
	
}

uint8_t u8_calculate_charge_percent(int16_t i16_voltage_x4)
{
	// voltageinput is 12bit, 4096 = 3.6V at AIN0
	uint32_t u32_calc_var = i16_voltage_x4;
	
	// invert resistance divisor
	u32_calc_var = u32_calc_var * 32;
	u32_calc_var = u32_calc_var / 22;
	
	// now, 4096 equals 5.2364V, 1 bit equals 1.278mV
	
	// simple algorithm, linear: 100% = 4V, 20% 3.5V, 0% = 3V
	// 4V   : 3129
	// 3.5V : 2738
	// 3V   : 2347
	
	if(u32_calc_var >= 3129)
		return 100;
	else if(u32_calc_var >= 2738)
		return (u32_calc_var - 2738) / 5 + 20;
	else if(u32_calc_var >= 2347)
		return (u32_calc_var - 2347) / 20;
	else
		return 0;
}



