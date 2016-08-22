//Personal library
#include <SmartVision_V1.h>
#include <OV7675_dcmi.h>

// Global variables
extern uint8_t data[];
extern uint32_t data_size;
extern DCMI_HandleTypeDef DCMI_Handle;
extern I2C_StatusTypeDef I2C_Status;

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
	
	// Microcontroller Init
	HAL_Init();
	SystemClock_Config();
	
	// Port COM Init
	USBD_Init(&USBD_Device, &VCP_Desc, 0);
	USBD_RegisterClass(&USBD_Device, &USBD_CDC);
	USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_SmartVision_V1_fops);
	USBD_Start(&USBD_Device);
	while (!g_VCPInitialized) {} // Wait for the initialisation of thr VCP

	scanf("%d", &start); // Wait that the port COM is opened by the user
	
	OV7675_Init();
	
	// Color bar
//	I2C_Status = OV7675_Write_Reg_Bit(OV7675_COM7, 1, 1);
//	I2C_Status_Printf("Color bar", I2C_Status);
	
	while (1)
	{
		capture = DCMI_Handle.Instance->CR & (0x00000001);
		if (capture == 0)
		{
			asm("bkpt 255");
			OV7675_Send_Data(data, data_size);
			asm("bkpt 255");
		}
	}
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
