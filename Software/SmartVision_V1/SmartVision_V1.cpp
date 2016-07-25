//Personal library
#include <SmartVision_V1.h>
#include <OV7675_dcmi.h>

// Mavlink
#include <mavlink.h>
#include "mavlink_communication.h"
#include "settings_parameters.h"

// Global variables
	//DCMI
extern uint8_t image[];
extern uint32_t image_size;
extern uint16_t image_row;
extern uint16_t image_column;
extern DCMI_HandleTypeDef DCMI_Handle;
	//I2C
extern I2C_StatusTypeDef I2C_Status;
	//MAVLINK
uint8_t system_type = MAV_TYPE_FIXED_WING;
uint8_t autopilot_type = MAV_AUTOPILOT_GENERIC; 
uint8_t system_mode = MAV_MODE_PREFLIGHT; ///< Booting up
uint32_t custom_mode = 0;                 ///< Custom mode, can be defined by user/adopter
uint8_t system_state = MAV_STATE_STANDBY; ///< System ready for flight
mavlink_message_t msg;
uint8_t buf[MAVLINK_MAX_PACKET_LEN];

#ifdef __cplusplus
extern "C"
{
#endif
#include <usbd_core.h>
#include <usbd_cdc.h>
#include "usbd_cdc_if.h"
#include <usbd_desc.h>

	USBD_HandleTypeDef USBD_Device;
	void SysTick_Handler(void);
	void OTG_FS_IRQHandler(void);
	void OTG_HS_IRQHandler(void);
	extern PCD_HandleTypeDef hpcd;
	
	int VCP_read(void *pBuffer, int size);
	int VCP_write(const void *pBuffer, int size);
	extern char g_VCPInitialized;
	
#ifdef __cplusplus
}
#endif

#include<sys/stat.h>

extern"C" // To allow printf on COM port
{
	int _fstat(int fd, struct stat *pStat)
	{
		pStat->st_mode = S_IFCHR;
		return 0;
	}

	int _close(int)
	{
		return -1;
	}

	int _write(int fd, char *pBuffer, int size)
	{
		return VCP_write(pBuffer, size);
	}

	int _isatty(int fd)
	{
		return 1;
	}

	int _lseek(int, int, int)
	{
		return -1;
	}

	int _read(int fd, char *pBuffer, int size)
	{
		for (;;)
		{
			int done = VCP_read(pBuffer, size);
			if (done)
				return done;
		}
	}
}

static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	__PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK |
	                               RCC_CLOCKTYPE_HCLK |
	                               RCC_CLOCKTYPE_PCLK1 |
	                               RCC_CLOCKTYPE_PCLK2);

	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
	
}


void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

#ifdef USE_USB_FS
void OTG_FS_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&hpcd);
}
#elif defined(USE_USB_HS)
void OTG_HS_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&hpcd);
}
#else
#error USB peripheral type not defined
#endif



int main(void)
{
	char start = 0;
	uint8_t dataRead;
	uint8_t capture;
	uint32_t tick_start;
	
	// Microcontroller Init
	HAL_Init();
	SystemClock_Config();
	
	// Port COM Init
	USBD_Init(&USBD_Device, &VCP_Desc, 0);
	USBD_RegisterClass(&USBD_Device, &USBD_CDC);
	USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_SmartVision_V1_fops);
	USBD_Start(&USBD_Device);
	while (!g_VCPInitialized) {} // Wait for the initialisation of the VCP

//	scanf("%d", &start); // Wait that the port COM is opened by the user. 
						// Warning don't use this function when you want to use QGroundControl because it won't see the COM port !!
	
	OV7675_Init(QQVGA);
	
	// Color bar
	I2C_Status = OV7675_Write_Reg_Bit(OV7675_COM7, 1, 1);
	I2C_Status_Printf("Color bar", I2C_Status);
	
	global_data_reset_param_defaults();
	mavlink_communication_init();
	
	
	uint8_t * p_image = image;
	
	tick_start = HAL_GetTick();
	
	while (1)
	{
		// Send heartbeat every 1000 ms !
		if (HAL_GetTick() - tick_start > 1000)
		{
			mavlink_send_state();
			tick_start = HAL_GetTick();
		}		
		
		communication_receive();
		communication_parameter_send();
		
		//Wait the image
//		capture = DCMI_Handle.Instance->CR & (0x00000001);
//		if (capture == 0)
//		{
//			send_calibration_image(&p_image, image_size, image_row, image_column);
//			
//		}
	}
	
	
	
//	while (1)
//	{
//		capture = DCMI_Handle.Instance->CR & (0x00000001);
//		if (capture == 0)
//		{
//			//asm("bkpt 255");
//			//OV7675_Send_Data(data, data_size);
//			//asm("bkpt 255");
//		}
//	}
}

void LED_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	__HAL_RCC_GPIOD_CLK_ENABLE(); // Clock port D enable

	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_12;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void LED_Toogle()
{
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_SET);	
}







