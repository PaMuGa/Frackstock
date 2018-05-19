#include "bluetooth.h"
#include "nordic_common.h"
#include "nrf.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_hci.h"
#include "nrf_ble_gatt.h"
#include "ble_nus.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_util_platform.h"
#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define APP_BLE_CONN_CFG_TAG            1                                           /**< A tag identifying the SoftDevice BLE configuration. */
#define APP_BLE_CONN_CFG_TAG_MASTER		2
#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

/* Configuration */
#define DEVICE_NAME					"Frackstock"
#define NUS_SERVICE_UUID_TYPE       BLE_UUID_TYPE_VENDOR_BEGIN	// UUID type for the Nordic UART Service

#define APP_BLE_OBSERVER_PRIO       3

#define APP_ADV_INTERVAL			800							// *0.625ms => 500ms
#define APP_ADV_TIMEOUT				180							// seconds

#define MASTER_ADV_INTERVAL			320							// *0.625ms => 200ms
#define MASTER_ADV_TIMEOUT			1							// seconds

#define MIN_CONN_INTERVAL           MSEC_TO_UNITS(20, UNIT_1_25_MS)	// 20ms
#define MAX_CONN_INTERVAL           MSEC_TO_UNITS(75, UNIT_1_25_MS) // 75ms
#define SLAVE_LATENCY               0
#define CONN_SUP_TIMEOUT            MSEC_TO_UNITS(4000, UNIT_10_MS) // supervisory timeout (4s)

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define MASTER_BEACON_INFO_LENGTH			0x0E					// data length
#define MASTER_DEVICE_TYPE                 	0x02					// manufactor specific data
#define MASTER_DATA_LENGTH					0x0C					// manufactor data length
#define MASTER_IDENTIFICATION				0xC0, 0xB8, 0x89, 0xB6	// identification data 
#define FIRMWARE_VERSION					0x01
#define APP_COMPANY_IDENTIFIER          	0x0059                  /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */

#define MASTER_BEACON_OFFSET				0x07					// Ofset in data packet (advertising)


BLE_NUS_DEF(m_nus);                 		// BLE NUS service instance.
NRF_BLE_GATT_DEF(m_gatt);           		// GATT module instance.
BLE_ADVERTISING_DEF(m_advertising); 		// Advertising module instance.
BLE_ADVERTISING_DEF(master_advertising);	// Advertising module for master control

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; 	// Handle of the current connection.

static ble_gap_adv_params_t m_adv_params;		// Advertising parameters

ble_data_received_handler_t p_data_received_handler = NULL;		// BLE data received handler
ble_adv_timeout_handler_t p_adv_timeout_handler = NULL;			// Advertising timeout handler
ble_connection_handler_t p_connection_handler = NULL;			// BLE connected / disconnected handler
ble_slave_received_handler_t p_slave_received_handler = NULL;	// Slave data received handler

static void gap_params_init(void);
static void services_init(void);
static void conn_params_init(void);
static void ble_stack_init(void);
void gatt_init(void);
static void advertising_init(void);

void bluetooth_init(ble_data_received_handler_t data_received_handler,
					ble_adv_timeout_handler_t adv_timeout_handler,
					ble_connection_handler_t connection_handler)
{
	uint32_t err_code;
	
	p_data_received_handler = data_received_handler;
	p_adv_timeout_handler = adv_timeout_handler;
	p_connection_handler = connection_handler;

	err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
	
	ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
}


void bluetooth_start_advertising(void)
{
	uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}


static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

void bluetooth_send(uint8_t *u8_buffer, uint16_t u16_length)
{
	ble_nus_string_send(&m_nus, u8_buffer, &u16_length);
}

static void nus_data_handler(ble_nus_evt_t * p_evt)
{
    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
		#ifdef DEBUG
		NRF_LOG_INFO("Received data from BLE NUS");
		#endif
		
		if(p_data_received_handler != NULL)
		{
			p_data_received_handler(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
		}
    }
}

// initializes services
static void services_init(void)
{
    uint32_t       err_code;
    ble_nus_init_t nus_init;

    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}


static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            break;
        case BLE_ADV_EVT_IDLE:
			if(p_adv_timeout_handler != NULL)
			{
				p_adv_timeout_handler();
			}
            break;
        default:
            break;
    }
}


static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code;
	ble_gap_evt_adv_report_t adv_report;
	uint8_t u8_selected_pattern;
	uint32_t u32_control_state;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
			#ifdef DEBUG
            NRF_LOG_INFO("Connected");
			#endif
            //err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            //APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
		
			if(p_connection_handler != NULL)
				p_connection_handler(1);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
			#ifdef DEBUG
            NRF_LOG_INFO("Disconnected");
			#endif
            // LED indication will be changed when advertising starts.
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
		
			if(p_connection_handler != NULL)
				p_connection_handler(0);
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;
#if !defined (S112)
         case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
        {
            ble_gap_data_length_params_t dl_params;

            // Clearing the struct will effectivly set members to @ref BLE_GAP_DATA_LENGTH_AUTO
            memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
            err_code = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
            APP_ERROR_CHECK(err_code);
        } break;
#endif //!defined (S112)
		
		 case BLE_GAP_EVT_ADV_REPORT:
			// receiving slave control packets 
			adv_report = p_ble_evt->evt.gap_evt.params.adv_report;
			if(p_slave_received_handler != NULL)
			{
				if(adv_report.dlen == MASTER_BEACON_INFO_LENGTH + MASTER_BEACON_OFFSET)
				{
					if(adv_report.data[6 + MASTER_BEACON_OFFSET] == FIRMWARE_VERSION &&
						adv_report.data[2 + MASTER_BEACON_OFFSET] == 0xC0 &&
						adv_report.data[3 + MASTER_BEACON_OFFSET] == 0xB8 &&
						adv_report.data[4 + MASTER_BEACON_OFFSET] == 0x89 &&
						adv_report.data[5 + MASTER_BEACON_OFFSET] == 0xB6)
					{
						u8_selected_pattern = adv_report.data[7 + MASTER_BEACON_OFFSET];
						u32_control_state = adv_report.data[8 + MASTER_BEACON_OFFSET] << 24 |
							adv_report.data[9 + MASTER_BEACON_OFFSET] << 16 |
							adv_report.data[10 + MASTER_BEACON_OFFSET] << 8 |
							adv_report.data[11 + MASTER_BEACON_OFFSET];
						
						p_slave_received_handler(u8_selected_pattern, u32_control_state);
					}
				}
			}
			break;
		
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

        default:
            // No implementation needed.
            break;
    }
}

static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        //m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
    }
}


/**@brief Function for initializing the GATT library. */
void gatt_init(void)
{	
	ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, BLE_GATT_ATT_MTU_DEFAULT);
    APP_ERROR_CHECK(err_code);
}

static void advertising_init(void)
{
    uint32_t               err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_TIMEOUT;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


static uint8_t m_beacon_info[MASTER_BEACON_INFO_LENGTH] =
{
	MASTER_DEVICE_TYPE,
	MASTER_DATA_LENGTH,			// data length
	MASTER_IDENTIFICATION,		// to identify
	FIRMWARE_VERSION,			// Firmware Version
	0x00,						// selected pattern
	0x00, 0x00, 0x00, 0x00,		// control state info
	0x00, 0x00					// reserved for future use
};

ble_advdata_t advdata_master;
uint8_t       flags_master = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
ble_advdata_manuf_data_t manuf_specific_data_master;

void update_master_params(uint8_t *u8_pattern, uint32_t *params)
{
	uint32_t      err_code;
	
	m_beacon_info[7] = *u8_pattern;
	m_beacon_info[8] = *params >> 24;
	m_beacon_info[9] = (*params >> 16) & 0xFF;
	m_beacon_info[10] = (*params >> 8) & 0xFF;
	m_beacon_info[11] = *params & 0xFF;
	
	err_code = ble_advdata_set(&advdata_master, NULL);
    APP_ERROR_CHECK(err_code);
}

static uint8_t u8_master_advertising = 0;
static uint8_t u8_slave_scanning = 0;

void master_advertising_init(void)
{
	uint32_t      err_code;
	
	if(u8_master_advertising) return;
    
    manuf_specific_data_master.company_identifier = APP_COMPANY_IDENTIFIER;
	
	manuf_specific_data_master.data.p_data = (uint8_t *) m_beacon_info;
    manuf_specific_data_master.data.size   = MASTER_BEACON_INFO_LENGTH;

    // Build and set advertising data.
    memset(&advdata_master, 0, sizeof(advdata_master));

    advdata_master.name_type             = BLE_ADVDATA_NO_NAME;
    advdata_master.flags                 = flags_master;
    advdata_master.p_manuf_specific_data = &manuf_specific_data_master;

    err_code = ble_advdata_set(&advdata_master, NULL);
    APP_ERROR_CHECK(err_code);

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
    m_adv_params.p_peer_addr = NULL;    // Undirected advertisement.
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = MASTER_ADV_INTERVAL;
    m_adv_params.timeout     = 0;       // Never time out.
	
	err_code = sd_ble_gap_adv_start(&m_adv_params, APP_BLE_CONN_CFG_TAG_MASTER);
    APP_ERROR_CHECK(err_code);
	
	u8_master_advertising = 1;
}

void master_advertising_stop(void)
{
	if(u8_master_advertising)
	{
		uint32_t err_code = sd_ble_gap_adv_stop();
		u8_master_advertising = 0;
		APP_ERROR_CHECK(err_code);
	}
}

static ble_gap_scan_params_t scan_params;

void slave_scan_init(ble_slave_received_handler_t handler)
{
	uint32_t err_code;
	
	if(u8_slave_scanning) return;
	
	p_slave_received_handler = handler;
	
	scan_params.active = 0;
	scan_params.adv_dir_report = 0;
	scan_params.use_whitelist = 0;
	scan_params.interval = 160; // 100ms
	scan_params.window = 80;	// 50ms
	scan_params.timeout = 0; // never
	
	err_code = sd_ble_gap_scan_start(&scan_params);
	APP_ERROR_CHECK(err_code);
	
	u8_slave_scanning = 1;
}

void slave_scan_stop(void)
{
	if(u8_slave_scanning)
	{
		uint32_t err_code = sd_ble_gap_scan_stop();
		u8_slave_scanning = 0;
		APP_ERROR_CHECK(err_code);
	}
}

void bluetooth_disable(void)
{
	uint32_t err_code = nrf_sdh_disable_request();
	APP_ERROR_CHECK(err_code);
}
