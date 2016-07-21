#include "mavlink_communication.h"

mavlink_system_t mavlink_system;

/**
 * @brief Initialize the Mavlink communication by putting parameters into mavlink_system_t structure
 */
void mavlink_communication_init(void)
{
	mavlink_system.sysid = 20;                   ///< ID 20 for this airplane
	mavlink_system.compid = MAV_COMP_ID_IMU;     ///< The component sending the message is the IMU, it could be also a Linux process
}

/**
 * @brief Send heartbeat to Mavlink station
 */
void mavlink_send_state(void)
{
	mavlink_msg_heartbeat_send(MAVLINK_COMM_0, MAV_TYPE_GENERIC, MAV_AUTOPILOT_GENERIC, 0, 0, 0);
}

/**
 * @brief Send multiple chars (uint8_t) over a comm channel
 * @param chan : MAVLink channel to use
 * @param ch : address of the table
 * @param length : of the table
 */
void mavlink_send_uart_bytes(mavlink_channel_t chan, const uint8_t * ch, uint16_t length)
{
	for (int i = 0; i < length; i++)
	{
		VCP_write((ch+i), 1);
	}
}