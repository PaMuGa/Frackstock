#include "leds.h"
#include "custom_board.h"
#include "nrf.h"
#include "app_error.h"
#include "app_pwm.h"
#include "nrf_drv_pwm.h"
#include "nrf_drv_spis.h"
#include "nrf_delay.h"

APP_PWM_INSTANCE(PWM1,1);                   // Create the instance "PWM1" using TIMER1.

#define SPIS_INSTANCE 1
static const nrf_drv_spis_t spis = NRF_DRV_SPIS_INSTANCE(SPIS_INSTANCE);
static uint8_t m_tx_buf[3 * 60 * 2];
static uint8_t m_rx_buf[3 * 60 * 2];

uint8_t first_or_second = 1;
uint16_t demo_counter = 0;

void enable_led_pwm(void);


/**
 * @brief SPIS user event handler.
 *
 * @param event
 */
void spis_event_handler(nrf_drv_spis_event_t event)
{
    if (event.evt_type == NRF_DRV_SPIS_XFER_DONE)
    {
		app_pwm_disable(&PWM1);
		
		if(first_or_second == 1)
		{
			// send second block
			APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_tx_buf + 256, 105, m_rx_buf, 105));
			first_or_second = 2;
			
			enable_led_pwm();
		}
		else
		{
			// leds go active
			nrf_delay_ms(1);
			
			// prepare next colors...
			m_tx_buf[demo_counter] = 0;
			demo_counter = (demo_counter +1) % 360;
			m_tx_buf[demo_counter] = 255;
			
			APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_tx_buf, 255, m_rx_buf, 255));
			first_or_second = 1;
			
			enable_led_pwm();
		}
    }
}


void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
	//nrf_gpio_cfg_output(17);
	//nrf_gpio_pins_set(1 << 17);
	//nrf_gpio_pins_clear(1 << 17);
}

void leds_init(void)
{
	nrf_drv_spis_config_t spis_config;
	
	spis_config.miso_drive = NRF_DRV_SPIS_DEFAULT_MISO_DRIVE;
	spis_config.csn_pullup = NRF_DRV_SPIS_DEFAULT_CSN_PULLUP;
	spis_config.orc = 0;
	spis_config.def = 0;
	spis_config.mode         = (nrf_drv_spis_mode_t)0;
    spis_config.bit_order    = (nrf_drv_spis_endian_t)0;
    spis_config.irq_priority = 3;
    spis_config.csn_pin               = PIN_SPI_CSN;
    spis_config.miso_pin              = PIN_SPI_SPID;
    spis_config.mosi_pin              = NRF_DRV_SPIS_PIN_NOT_USED;
    spis_config.sck_pin               = PIN_SPI_SCK;

    APP_ERROR_CHECK(nrf_drv_spis_init(&spis, &spis_config, spis_event_handler));
	APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, m_tx_buf, 255, m_rx_buf, 255));
	
	
	ret_code_t err_code;
	/* 2-channel PWM, 200 Hz, output on DK LED pins. */
	app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_2CH(1, PIN_PWM1, PIN_PWM2);
	/* Switch the polarity of the second channel. */
	pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;
	pwm1_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;
	/* Initialize and enable PWM. */
	err_code = app_pwm_init(&PWM1,&pwm1_cfg, pwm_ready_callback);
	APP_ERROR_CHECK(err_code);
	
	nrf_gpio_cfg(PIN_PWM1, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_D0H1, NRF_GPIO_PIN_NOSENSE);
	nrf_gpio_cfg(PIN_PWM2, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_H0D1, NRF_GPIO_PIN_NOSENSE);
	nrf_gpio_cfg(PIN_SPI_SPID, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_H0D1, NRF_GPIO_PIN_NOSENSE);
	
	// led power supply, config and disable
	nrf_gpio_cfg_output(PIN_LED_EN);
	nrf_gpio_pin_clear(PIN_LED_EN);
	
	
}


void leds_activate(void)
{
	// Enable power supply
	nrf_gpio_pin_set(PIN_LED_EN);
	
	enable_led_pwm();
}


void enable_led_pwm(void)
{
	app_pwm_enable(&PWM1);
	while (app_pwm_channel_duty_set(&PWM1, 1, 62) == NRF_ERROR_BUSY);
	// pwm running after this while!!!
	while (app_pwm_channel_duty_set(&PWM1, 0, 38) == NRF_ERROR_BUSY);
	
	app_pwm_enable(&PWM1);
}

void leds_deactivate(void)
{
	// disable power supply
	nrf_gpio_pin_clear(PIN_LED_EN);
	
	app_pwm_disable(&PWM1);
}
