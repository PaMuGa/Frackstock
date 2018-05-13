#include "lis3de.h"
#include "lis3de_registermap.h"

#include "custom_board.h"

#include "nrf.h"
#include "nrf_drv_twi.h"
#include "nrf_drv_spis.h"
#include "nrf_delay.h"

#include "nrf_log.h"

#define LIS3DE_ADDR			0x29
#define WHO_AM_I_DEFAULT	0x33

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(0);

/* Buffer for samples read. */
static uint8_t m_sample;
static LIS3DE_REGISTER_t last_read_register;
static uint8_t init_finished = 0;

static data_handler_t p_data_handler;
static xyz_data_handler_t p_xyz_data_handler;

static const uint8_t CONFIG_DATA[] = {0x5F, 0x00, 0x10, 0x00, 0x00, 0x00};

// REG1: 100Hz, Lowpower mode, XYZ enabled
// REG2: No filter enabled
// REG3: Data ready interrupt on int1
// REG4: +- 2g
// REG5: default
// REG6: default

typedef enum {
	IDLE,
	WAIT_X,
	WAIT_Y,
	WAIT_Z
} READ_XYZ_t;

READ_XYZ_t read_xyz_state = IDLE;

static lis3de_xyz_acc_data_t acc_data_xyz;


/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
	NRF_LOG_INFO("LIS3DE handler.");
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
            {
				if(last_read_register == LIS3DE_REG_WHO_AM_I)
				{
					if(m_sample == WHO_AM_I_DEFAULT)
					{
						nrf_drv_twi_tx(&m_twi, LIS3DE_REG_CTRL_REG1, CONFIG_DATA, sizeof(CONFIG_DATA), false);
						init_finished = 1;
						NRF_LOG_INFO("LIS3DE initialized.");
					}
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
			NRF_LOG_WARNING("LIS3DE no connection.");
			break;
		case NRF_DRV_TWI_EVT_DATA_NACK:
			NRF_LOG_WARNING("LIS3DE something wrong.");
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
	
	nrf_delay_us(10);
	
	last_read_register = LIS3DE_REG_WHO_AM_I;
	m_sample = (char)last_read_register;
	
	APP_ERROR_CHECK(nrf_drv_twi_rx(&m_twi, LIS3DE_ADDR, &m_sample, sizeof(m_sample)));
}


int lis3de_read_async(LIS3DE_REGISTER_t lis3de_register, data_handler_t data_handler)
{
	//if(init_finished == 1)
	//{
		m_sample = (char)lis3de_register;
		last_read_register = lis3de_register;
		p_data_handler = data_handler;
	
		nrf_drv_twi_rx(&m_twi, LIS3DE_ADDR, &m_sample, sizeof(m_sample));
		return 0;
//	}
//	else
//	{
//		NRF_LOG_INFO("LIS3DE connection not (correct) initialized.");
//		return 1;
//	}
}

void lis3de_xyz_data_handler(LIS3DE_REGISTER_t lis3de_register, uint8_t data)
{
	switch(lis3de_register)
	{
		case LIS3DE_REG_OUT_X:
			acc_data_xyz.acc_x = data;
			read_xyz_state = WAIT_Y;
			lis3de_read_async(LIS3DE_REG_OUT_Y, lis3de_xyz_data_handler);
			break;
		case LIS3DE_REG_OUT_Y:
			acc_data_xyz.acc_y = data;
			read_xyz_state = WAIT_Z;
			lis3de_read_async(LIS3DE_REG_OUT_Z, lis3de_xyz_data_handler);
			break;
		case LIS3DE_REG_OUT_Z:
			acc_data_xyz.acc_z = data;
			read_xyz_state = IDLE;
			if(p_xyz_data_handler != NULL)
			{
				p_xyz_data_handler(acc_data_xyz);
			}
			break;
		default:
			break;
	}
}

int lis3de_read_XYZ_async(xyz_data_handler_t xyz_data_handler)
{
	if(init_finished != 1)
		return 1;
	
	if(read_xyz_state == IDLE)
	{
		p_xyz_data_handler = xyz_data_handler;
		read_xyz_state = WAIT_X;
		lis3de_read_async(LIS3DE_REG_OUT_X, lis3de_xyz_data_handler);
		return 0;
	}
	return 2;
}


