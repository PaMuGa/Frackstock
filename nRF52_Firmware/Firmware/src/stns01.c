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


uint8_t stns01_get_charge(void)
{
	//charge_measured_callback = callback;
	static nrf_saadc_value_t adc_value[4];
	static uint8_t u8_adc_index = 0;
	
	//nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
	nrf_drv_saadc_sample_convert(0, &adc_value[u8_adc_index]);
	u8_adc_index++;
	u8_adc_index %= 4;
	
	return u8_calculate_charge_percent(adc_value[0] + adc_value[1] + adc_value[2] + adc_value[3]);
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
	// 4V   : 4549
	// 3.5V : 3979
	// 3V   : 3752
	
	if(u32_calc_var >= 4549)
		return 100;
	else if(u32_calc_var >= 3979)
		return (u32_calc_var - 3979) / 5 + 20;
	else if(u32_calc_var >= 3752)
		return (u32_calc_var - 3752) / 20;
	else
		return 0;
}



