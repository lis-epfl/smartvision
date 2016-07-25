#include <mavlink.h>
#include "settings_parameters.h"

enum global_param_id_t global_param_id;
struct global_struct global_data;

void global_data_reset_param_defaults(void)
{
	global_data.param[PARAM_SYSTEM_ID] = SYSTEM_ID;
	strcpy(global_data.param_name[PARAM_SYSTEM_ID], "SYS_ID");
	global_data.param_access[PARAM_SYSTEM_ID] = READ_WRITE;
	
	global_data.param[PARAM_COMPONENT_ID] = COMPONENT_ID;
	strcpy(global_data.param_name[PARAM_COMPONENT_ID], "SYS_COMP_ID");
	global_data.param_access[PARAM_COMPONENT_ID] = READ_WRITE;
	
	global_data.param[LEUG] = 0;
	strcpy(global_data.param_name[LEUG], "LEUG");
	global_data.param_access[LEUG] = READ_WRITE;
	
	
}
