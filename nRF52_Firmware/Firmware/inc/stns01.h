#ifndef __stns01_h
#define __stns01_h

#include <stdint.h>

typedef void (*stns01_charge_measured_handler_t)(uint8_t);

void stns01_init(void);
uint8_t stns01_get_charge(void);
uint8_t stns01_get_charging_state(void);

/**
* @brief Calculates the battery voltage in mV based on the measurements
*        of stns01_get_charge().
*/
uint16_t stns01_get_battery_voltage(void);

#endif /* __stns01_h */
