#include "stdint.h"

#ifndef __bluetooth_h
#define __bluetooth_h

typedef void (*ble_data_received_handler_t)(const uint8_t* p_data, uint8_t length);
typedef void (*ble_adv_timeout_handler_t)(void);
typedef void (*ble_connection_handler_t)(uint8_t);
typedef void (*ble_slave_received_handler_t)(uint8_t u8_pattern, uint32_t u32_control_state);

/**
* @brief Initialize Bluetooth and start advertising
*/
void bluetooth_init(ble_data_received_handler_t data_received_handler,
					ble_adv_timeout_handler_t adv_timeout_handler,
					ble_connection_handler_t connection_handler);

void bluetooth_start_advertising(void);

/**
* @brief Send data over Bluetooth
*/
void bluetooth_send(uint8_t *u8_buffer, uint16_t u16_length);

/**
* @brief Update parameters for master control
*/
void update_master_params(uint8_t *u8_pattern, uint32_t *params);

/**
* @brief Initialize advertising for master control
*/
void master_advertising_init(void);

/**
* @brief Stop adverising for master control
*/
void master_advertising_stop(void);

/**
* @brief Initialize advertising scan for slave
*/
void slave_scan_init(ble_slave_received_handler_t);

/**
* @brief Stop advertising scan for slave
*/
void slave_scan_stop(void);

/**
* @brief Disable soft-device for entering sleep mode
*/
void bluetooth_disable(void);


#endif /* __bluetooth_h */
