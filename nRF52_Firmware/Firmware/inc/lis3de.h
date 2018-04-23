#include "lis3de_registermap.h"

#ifndef LIS3DE_H
#define LIS3DE_H

typedef void (*data_handler_t)(LIS3DE_REGISTER_t, uint8_t);

void lis3de_init(void);
int lis3de_read_async(LIS3DE_REGISTER_t, data_handler_t);


#endif /* LIS3DE_H */
