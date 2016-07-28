/******************************************************************************
 * @file     settings_parameters.cpp
 * @brief    Settings the Mavlink parameters for QGround Control
 * @version  V1
 * @date     28. July 2016
 * @Author	 W. PONSOT
 ******************************************************************************/

#include "settings_parameters.h"

enum global_param_id_t global_param_id;
struct global_struct global_data;

/* See information in settings_parameters.h to add other parameters */
void global_data_reset_param_defaults(void)
{
	global_data.param[PARAM_SYSTEM_ID] = SYSTEM_ID;
	strcpy(global_data.param_name[PARAM_SYSTEM_ID], "SYS_ID");
	global_data.param_access[PARAM_SYSTEM_ID] = READ_ONLY;
	
	global_data.param[PARAM_COMPONENT_ID] = COMPONENT_ID;
	strcpy(global_data.param_name[PARAM_COMPONENT_ID], "SYS_COMP_ID");
	global_data.param_access[PARAM_COMPONENT_ID] = READ_ONLY;
	
	global_data.param[C_IMAGE] = 0;
	strcpy(global_data.param_name[C_IMAGE], "C_IMAGE");
	global_data.param_access[C_IMAGE] = READ_WRITE;
	
	global_data.param[C_COM7] = 0;
	strcpy(global_data.param_name[C_COM7], "C_COM7");
	global_data.param_access[C_COM7] = READ_WRITE;
	
	global_data.param[C_COM8] = 0;
	strcpy(global_data.param_name[C_COM8], "C_COM8");
	global_data.param_access[C_COM8] = READ_WRITE;
	
	global_data.param[C_COM9] = 0;
	strcpy(global_data.param_name[C_COM9], "C_COM9");
	global_data.param_access[C_COM9] = READ_WRITE;
	
	global_data.param[C_GAIN] = 0;
	strcpy(global_data.param_name[C_GAIN], "C_GAIN");
	global_data.param_access[C_GAIN] = READ_WRITE;
	
	global_data.param[TEST] = 0;
	strcpy(global_data.param_name[TEST], "TEST");
	global_data.param_access[TEST] = READ_WRITE;
}
