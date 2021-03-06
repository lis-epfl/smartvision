/******************************************************************************
 * @file     mavlink_bridge_header.h
 * @brief    Make the link between the Mavlink library and our codes
 * @version  V1
 * @date     28. July 2016
 * @Author	 W. PONSOT
 ******************************************************************************/

#ifndef MAVLINK_BRIDGE_HEADER_H
#define MAVLINK_BRIDGE_HEADER_H

// Allow to use higher level Mavlink functions
#define MAVLINK_USE_CONVENIENCE_FUNCTIONS

// Define the final function to send data through serial port
#define MAVLINK_SEND_UART_BYTES mavlink_send_uart_bytes

#include "mavlink_types.h"

extern mavlink_system_t mavlink_system;

/* defined in mavlink_communication.cpp */
extern void mavlink_send_uart_bytes(mavlink_channel_t chan, const uint8_t * ch, uint16_t length);

#endif