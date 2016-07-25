#include "mavlink_communication.h"
#include "settings_parameters.h"

mavlink_system_t mavlink_system;
static uint32_t m_parameter_i = 0;

/**
 * @brief Send an image to QGround control station through serial port.  !! WARNING : for now wrong data format !! --> RAW8U ???
 * @param image : pointer on the address of the beginning of the table
 * @param image_size : size of the table (bytes)
 * @param image_row : number of row
 * @param image_column : number of column
 */
void send_calibration_image(uint8_t ** image, uint32_t image_size, uint16_t image_row, uint16_t image_column )
{
	uint16_t frame = 0;
	uint8_t frame_buffer[MAVLINK_MSG_ENCAPSULATED_DATA_FIELD_DATA_LEN];
		
	mavlink_msg_data_transmission_handshake_send(
		MAVLINK_COMM_0,
		MAVLINK_DATA_STREAM_IMG_RAW8U, // Only luma part of the image is sended --> Y1Y2Y3...Yn
		image_size,
		image_row*2,
		image_column,
		image_size / MAVLINK_MSG_ENCAPSULATED_DATA_FIELD_DATA_LEN + 1,
		MAVLINK_MSG_ENCAPSULATED_DATA_FIELD_DATA_LEN,
		100);
	
	for (int i = 0; i < image_size; i++)
	{
		if (i % MAVLINK_MSG_ENCAPSULATED_DATA_FIELD_DATA_LEN == 0 && i != 0)
		{
			mavlink_msg_encapsulated_data_send(MAVLINK_COMM_0, frame, frame_buffer);
			frame++;
			HAL_Delay(2);
			
		}
		frame_buffer[i % MAVLINK_MSG_ENCAPSULATED_DATA_FIELD_DATA_LEN] = (uint8_t)(*image)[i % image_size];
	}
	mavlink_msg_encapsulated_data_send(MAVLINK_COMM_0, frame, frame_buffer);
}

/**
 * @brief Initialize the Mavlink communication by putting parameters into mavlink_system_t structure
 */
void mavlink_communication_init(void)
{
	mavlink_system.sysid = SYSTEM_ID;                  
	mavlink_system.compid = COMPONENT_ID;   
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

/**
 * @brief Send one by one the parameters stored in global_data structure
 */
void communication_parameter_send(void)
{
	if (m_parameter_i < ONBOARD_PARAM_COUNT)
	{
		mavlink_msg_param_value_send(MAVLINK_COMM_0, 
			global_data.param_name[m_parameter_i], 
			global_data.param[m_parameter_i], MAVLINK_TYPE_FLOAT, ONBOARD_PARAM_COUNT, m_parameter_i);
		m_parameter_i++;
	}
}

/**
 * @brief Receive messages coming from QGround station
 */
void communication_receive(void)
{
	mavlink_message_t msg;
	mavlink_status_t status = { 0 };
	uint8_t character;
	
	while (VCP_read(&character, 1))
	{
		/* Try to get a new message */
		if (mavlink_parse_char(MAVLINK_COMM_0, character, &msg, &status))
		{
			handle_mavlink_message(MAVLINK_COMM_0, &msg);
		}
	}
}

/**
 * @brief Handle messages coming from QGround station
 */
void handle_mavlink_message(mavlink_channel_t chan, mavlink_message_t * msg)
{
	switch (msg->msgid)
	{
		case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
		{
			asm("bkpt 255");
		}
		break;
		
		case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
		{
			// Send all the parameters --> communication_parameter_send()
			m_parameter_i = 0;
		}
		break;
		
		case MAVLINK_MSG_ID_PARAM_SET:
		{
			mavlink_param_set_t set;
			mavlink_msg_param_set_decode(msg, &set);
			
			/* Check if this message is for this system */
			if ((uint8_t) set.target_system
					== (uint8_t) global_data.param[PARAM_SYSTEM_ID]
												   && (uint8_t) set.target_component
												   == (uint8_t) global_data.param[PARAM_COMPONENT_ID])
			{
				char* key = (char*) set.param_id;

				for (int i = 0; i < ONBOARD_PARAM_COUNT; i++)
				{
					bool match = true;
					for (int j = 0; j < ONBOARD_PARAM_NAME_LENGTH; j++)
					{
						/* Compare */
						if (((char)(global_data.param_name[i][j]))
								!= (char)(key[j]))
						{
							match = false;
						}

						/* End matching if null termination is reached */
						if (((char) global_data.param_name[i][j]) == '\0')
						{
							break;
						}
					}
					if (match)
					{
						// Only write if is not "not a number" AND if is not infinity AND if access is allowed
						if (!isnan(set.param_value) && !isinf(set.param_value) && global_data.param_access[i])
						{
							global_data.param[i] = set.param_value;
						}
					}
				}		
			}
		}
		break;
		
		case MAVLINK_MSG_ID_PING:
		{
			asm("bkpt 255");
		}
		break;	
		
		case MAVLINK_MSG_ID_COMMAND_LONG:
		{
			asm("bkpt 255");
		}
		break;
	}
}