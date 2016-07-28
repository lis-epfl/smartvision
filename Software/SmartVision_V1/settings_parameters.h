/******************************************************************************
 * @file     settings_parameters.h
 * @brief    Settings the Mavlink parameters for QGround Control
 * @version  V1
 * @date     28. July 2016
 * @Author	 W. PONSOT
 ******************************************************************************/

#ifndef SETTINGS_PARAMETERS_H
#define SETTINGS_PARAMETERS_H

#include <mavlink.h>
#include "OV7675_dcmi.h"
#include "OV7675_registers.h"

#define ONBOARD_PARAM_NAME_LENGTH 15
#define SYSTEM_ID 1
#define COMPONENT_ID 2

/**
  * @brief  parameter access
  */
typedef enum
{
	READ_ONLY  = 0,
	READ_WRITE = 1,
} ParameterAccess_TypeDef;

/* HOW TO ADD A "CLASSICAL" PARAMTER ?
 * Add the parameter name in enum global_param_id_t. The order is not important. For a classical parameter, the name can't start by "C_" !
 * Add the initialisation for this parameter in the function global_data_reset_param_defaults() by taking as example the other parameter.
 */

/* HOW TO ADD A CAMERA PARAMTER ?
 * Add the parameter name in enum global_param_id_t. The order is not important. The name MUST start by "C_"
 * Add the initialisation for this parameter in the function global_data_reset_param_defaults() by taking as example the other parameter.
 * Add a case in the functions write_camera_register() and read_camera_register() in the file mavlink_communication.cpp
 * These functions make the link between the Mavlink parameter and the camera register !
 */
enum global_param_id_t
{
	PARAM_SYSTEM_ID = 0,
	PARAM_COMPONENT_ID,
	
	// Mavlink Camera OmniVision
	C_IMAGE, //Used to take an other image without changing a register
	C_COM7, 
	C_COM8,
	C_COM9,
	C_GAIN,
	
	TEST,
	ONBOARD_PARAM_COUNT
};

struct global_struct
{
	float param[ONBOARD_PARAM_COUNT];
	char param_name[ONBOARD_PARAM_COUNT][ONBOARD_PARAM_NAME_LENGTH];
	ParameterAccess_TypeDef param_access[ONBOARD_PARAM_COUNT];
	
};


void global_data_reset_param_defaults(void);

	// Global declarations
extern enum global_param_id_t global_param_id;
extern struct global_struct global_data;

#endif