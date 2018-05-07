#ifndef __stns01_h
#define __stns01_h

#include <stdint.h>

typedef void (*stns01_charge_measured_handler_t)(uint8_t);

void stns01_init(void);
uint8_t stns01_get_charge(void);
uint8_t stns01_get_charging_state(void);

#endif /* __stns01_h */
