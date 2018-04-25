#include "lis3de_registermap.h"

#ifndef LIS3DE_H
#define LIS3DE_H

typedef struct
{
	int8_t acc_x;
	int8_t acc_y;
	int8_t acc_z;
} lis3de_xyz_acc_data_t;

typedef void (*data_handler_t)(LIS3DE_REGISTER_t, uint8_t);
typedef void (*xyz_data_handler_t)(lis3de_xyz_acc_data_t);

/**
* @brief	Initializes the I2C Interface and the LIS3DE
*/
void lis3de_init(void);

/**
* @brief	Reads the specified register register
*/
int lis3de_read_async(LIS3DE_REGISTER_t, data_handler_t);

/**
* @brief 	Start asynchronous reading of X, Y and Z acceleration
* @param	xyz_data_handler_t	Handler to be called after reading
* @return	0 if start reading successful, 1 if initialisation unfinished,
*			2 if readout still in progress
*/
int lis3de_read_XYZ_async(xyz_data_handler_t);


#endif /* LIS3DE_H */
