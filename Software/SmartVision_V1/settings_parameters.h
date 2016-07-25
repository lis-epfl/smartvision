




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

enum global_param_id_t
{
	PARAM_SYSTEM_ID = 0,
	PARAM_COMPONENT_ID,
	LEUG,
	
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