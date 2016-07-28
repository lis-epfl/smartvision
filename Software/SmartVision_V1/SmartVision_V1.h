/******************************************************************************
 * @file     SmartVision.h
 * @brief    Main file
 * @version  V1
 * @date     28. July 2016
 * @Author	 W. PONSOT
 ******************************************************************************/
#include <OV7675_dcmi.h>

	// Mavlink
#include <mavlink.h>
#include "mavlink_communication.h"
#include "settings_parameters.h"

	// Mavlink function
void mavlink_init();
void mavlink_send_heartbeat();


