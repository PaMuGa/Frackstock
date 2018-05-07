#include "stdint.h"

#ifndef __bluetooth_h
#define __bluetooth_h

typedef void (*ble_data_received_handler_t)(const uint8_t* p_data, uint8_t length);
typedef void (*ble_adv_timeout_handler_t)(void);
typedef void (*ble_connection_handler_t)(uint8_t);

void bluetooth_init(ble_data_received_handler_t data_received_handler,
					ble_adv_timeout_handler_t adv_timeout_handler,
					ble_connection_handler_t connection_handler);

void bluetooth_send(uint8_t *u8_buffer, uint16_t u16_length);

#endif /* __bluetooth_h */
