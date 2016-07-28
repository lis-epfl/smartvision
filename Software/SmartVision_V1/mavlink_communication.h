/******************************************************************************
 * @file     mavlink_communication.h
 * @brief    Functions to send (receive) datas to (from) QGround Control
 * @version  V1
 * @date     28. July 2016
 * @Author	 W. PONSOT
 ******************************************************************************/

#ifndef MAVLINK_COMMUNICATION_H
#define MAVLINK_COMMUNICATION_H

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "mavlink_bridge_header.h"
#include "mavlink.h"
#include "settings_parameters.h"
	// Camera
#include "OV7675_dcmi.h"
#include "OV7675_registers.h"

	// Extern USB function
extern "C"
{
	int VCP_write(const void *pBuffer, int size);
	int VCP_read(void *pBuffer, int size);
}

	// Initialization QGround Control
void mavlink_communication_init(void);

	// Send to QGround Control
void mavlink_send_state(void);
void send_calibration_image(uint8_t ** image, uint32_t image_size, uint16_t image_row, uint16_t image_column);
void communication_parameter_send(void);
void communication_send_specific_parameter(uint8_t param_ID);
void mavlink_send_uart_bytes(mavlink_channel_t chan, const uint8_t * ch, uint16_t length);

	// Receive from QGround Control
void communication_receive(void);
void handle_mavlink_message(mavlink_channel_t chan, mavlink_message_t * msg);

	// Camera 
I2C_StatusTypeDef write_camera_register(uint16_t name, float value);
I2C_StatusTypeDef read_camera_register(uint16_t name, uint8_t *value);

#endif