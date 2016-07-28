/******************************************************************************
 * @file     mavlink_communication.cpp
 * @brief    Functions to send (receive) datas to (from) QGround Control
 * @version  V1
 * @date     28. July 2016
 * @Author	 W. PONSOT
 ******************************************************************************/

#include "mavlink_communication.h"

mavlink_system_t mavlink_system;
static uint32_t m_parameter_i = 0;
char camera[] = "C_"; //Beginning of all the camera register

/*
 * @brief Send an image to QGround control station through serial port. 
 * The data are stored in image : u12 y1 v12 y2 u34 y3 v34 y4 ... (y:luminance, u and v chrominance). QGround will show a grayscale image !  
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
		MAVLINK_DATA_STREAM_IMG_RAW8U, // Only luminance part of the image is sended --> Y1Y2Y3...Yn
		image_size/2,
		image_column,
		image_row,
		image_size/2 / MAVLINK_MSG_ENCAPSULATED_DATA_FIELD_DATA_LEN + 1,
		MAVLINK_MSG_ENCAPSULATED_DATA_FIELD_DATA_LEN,
		100);
	
	for (int i = 0; i < image_size/2; i++)
	{
		if (i % MAVLINK_MSG_ENCAPSULATED_DATA_FIELD_DATA_LEN == 0 && i != 0)
		{
			mavlink_msg_encapsulated_data_send(MAVLINK_COMM_0, frame, frame_buffer);
			frame++;
		}
		frame_buffer[i % MAVLINK_MSG_ENCAPSULATED_DATA_FIELD_DATA_LEN] = (uint8_t)(*image)[(2*i + 1)];
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
 * @brief Send one by one the parameters stored in global_data structure. 
 * If it's a camera parameter, read first the value on the camera and send it to QGround Control.
 */
void communication_parameter_send(void)
{
	uint8_t value;
	
	if (m_parameter_i < ONBOARD_PARAM_COUNT)
	{
		bool match_camera = true;
		for (int i = 0; i < ONBOARD_PARAM_NAME_LENGTH; i++)
		{
			/* Compare if it's a camera parameter*/
			if (((char)(camera[i])
				!= (char)(global_data.param_name[m_parameter_i][i])) && i < strlen(camera))
			{
				match_camera = false;
			}
			/* End matching if null termination is reached */
			if (((char) global_data.param_name[m_parameter_i][i]) == '\0')
			{
				break;
			}
		}
		
		if (match_camera && m_parameter_i != C_IMAGE)
		{
			read_camera_register(m_parameter_i, &value);
			global_data.param[m_parameter_i] = (float) value;
 		}
		
		communication_send_specific_parameter(m_parameter_i);
		m_parameter_i++;
	}
}

/**
 * @brief Send one parameter stored in global_data structure
 * @param param_ID : identification number of the parameter declared in enum global_param_id_t (settings_parameters.h)
 */
void communication_send_specific_parameter(uint8_t param_ID)
{
	mavlink_msg_param_value_send(MAVLINK_COMM_0, 
		global_data.param_name[param_ID], 
		global_data.param[param_ID],
		MAVLINK_TYPE_FLOAT,
		ONBOARD_PARAM_COUNT,
		param_ID);
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
 * @param chan : MAVLink channel to use
 * @param msg : message decripted by mavlink_parse_char coming from QGround station
 */
void handle_mavlink_message(mavlink_channel_t chan, mavlink_message_t * msg)
{
	I2C_StatusTypeDef I2C_Status;
	float previous_value;
	
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
					bool match_camera = true;
					
					for (int j = 0; j < ONBOARD_PARAM_NAME_LENGTH; j++)
					{
						/* Compare if a parameter has the same name*/
						if (((char)(global_data.param_name[i][j]))
								!= (char)(key[j]))
						{
							match = false;
						}
						
						/* Compare if it's a camera parameter*/
						if (((char)(camera[j])
								!= (char)(key[j])) && j < strlen(camera))
						{
							match_camera = false;
						}
						
						/* End matching if null termination is reached */
						if (((char) global_data.param_name[i][j]) == '\0')
						{
							break;
						}
					}
					if (match)
					{
						/* Only write if is not "not a number" 
						AND if is not infinity 
						AND if access is allowed 
						AND if the value is different */
						if (global_data.param[i] != set.param_value 
							&& !isnan(set.param_value) 
							&& !isinf(set.param_value) 
							&& global_data.param_access[i])
						{
							if (match_camera)
							{
								global_data.param[C_IMAGE] = 1; // Allow to send another image each time a parameter of the camera is changed
								
								previous_value = global_data.param[i];
								global_data.param[i] = set.param_value;
								communication_send_specific_parameter(i);
								
								I2C_Status = write_camera_register(i, set.param_value);
								
								//If the register value can't be written, save the previous value and send it to QGround Control
								if (I2C_Status != I2C_OK)
								{
									global_data.param[i] = previous_value;
									communication_send_specific_parameter(i);
								}
							}
							else
							{
								global_data.param[i] = set.param_value;
								communication_send_specific_parameter(i); //QGround need to know that the parameter is received so it's sended back !
							}
							
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
//			asm("bkpt 255");
		}
		break;
	}
}

/**
 * @brief Write a camera register that has the "same" name as the variables defined in enum global_param_id_t
 * @param name : name of the register to change
 * @param value : value to put in the regiser
 * @retval I2C_Status : status of the I2C transfer
 */
I2C_StatusTypeDef write_camera_register(uint16_t name, float value)
{
	uint8_t param_value = (uint8_t) value;
	I2C_StatusTypeDef I2C_Status;
	
	switch (name)
	{
	case C_COM7:
		{
			I2C_Status = OV7675_Write_Reg(OV7675_COM7, &param_value);	
		}	
		break;
	case C_COM8:
		{
			I2C_Status = OV7675_Write_Reg(OV7675_COM8, &param_value);
		}
		break;
	case C_COM9:
		{
			I2C_Status = OV7675_Write_Reg(OV7675_COM9, &param_value);
		}
		break;
	case C_GAIN:
		{
			I2C_Status = OV7675_Write_Reg(OV7675_GAIN, &param_value);
		}
		break;
	}
	
	return I2C_Status;
}

/**
 * @brief Read a camera register that has the "same" name as the variables defined in enum global_param_id_t
 * @param name : name of the register to change
 * @param value : value read in the regiser
 * @retval I2C_Status : status of the I2C transfer
 */
I2C_StatusTypeDef read_camera_register(uint16_t name, uint8_t *value)
{
	I2C_StatusTypeDef I2C_Status;
	
	switch (name)
	{
	case C_COM7:
		{
			I2C_Status = OV7675_Read_Reg(OV7675_COM7, value);	
		}	
		break;
	case C_COM8:
		{
			I2C_Status = OV7675_Read_Reg(OV7675_COM8, value);
		}
		break;
	case C_COM9:
		{
			I2C_Status = OV7675_Read_Reg(OV7675_COM9, value);
		}
		break;
	case C_GAIN:
		{
			I2C_Status = OV7675_Read_Reg(OV7675_GAIN, value);
		}
		break;
	}
	
	return I2C_Status;
}
