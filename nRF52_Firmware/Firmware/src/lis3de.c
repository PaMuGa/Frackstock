#include "lis3de.h"
#include "lis3de_registermap.h"

#include "custom_board.h"

#include "nrf.h"
#include "nrf_drv_twi.h"
#include "nrf_drv_spis.h"

#include "nrf_log.h"

#define LIS3DE_ADDR			0x29
#define WHO_AM_I_DEFAULT	0x33

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(0);

/* Buffer for samples read. */
static uint8_t m_sample;
static LIS3DE_REGISTER_t last_read_register;
static uint8_t init_finished = 0;

data_handler_t p_data_handler;

/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
            {
				if(last_read_register == LIS3DE_REG_WHO_AM_I)
				{
					if(m_sample == WHO_AM_I_DEFAULT)
						init_finished = 1;
				}
				else
				{
					if(p_data_handler != NULL)
						p_data_handler(last_read_register, m_sample);
				}
            }
            //m_xfer_done = true;
            break;
		case NRF_DRV_TWI_EVT_ADDRESS_NACK:
			// Keine Verbindung. Chip korrekt angelötet?
			break;
		case NRF_DRV_TWI_EVT_DATA_NACK:
			// Verbindung ja, ungültiges Register / Daten
			break;
        default:
            break;
    }
}

void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_lis3de_config = {
       .scl                = PIN_SCL,
       .sda                = PIN_SDA,
       .frequency          = NRF_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_lis3de_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}


void lis3de_init(void)
{
	twi_init();
	
	last_read_register = LIS3DE_REG_WHO_AM_I;
	m_sample = (char)last_read_register;
	
	nrf_drv_twi_rx(&m_twi, LIS3DE_ADDR, &m_sample, sizeof(m_sample));
}


int lis3de_read_async(LIS3DE_REGISTER_t lis3de_register, data_handler_t data_handler)
{
	if(init_finished == 1)
	{
		m_sample = (char)lis3de_register;
		last_read_register = lis3de_register;
		p_data_handler = data_handler;
	
		nrf_drv_twi_rx(&m_twi, LIS3DE_ADDR, &m_sample, sizeof(m_sample));
		return 0;
	}
	else
	{
		NRF_LOG_INFO("LIS3DE connection not (correct) initialized.");
		return 1;
	}
}


