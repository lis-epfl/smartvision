#include "OV7675_dcmi.h"
#include "mavlink.h"

//// Private variables

I2C_HandleTypeDef I2C_Handle;
DCMI_HandleTypeDef DCMI_Handle;
DMA_HandleTypeDef DMA_Handle;

I2C_StatusTypeDef I2C_Status;
HAL_StatusTypeDef HAL_Status;

uint8_t data[QQVGA_YUV_size];
uint32_t data_size = QQVGA_YUV_size;

OV7675_IDTypeDef  OV7675_Camera_ID; 

// Varible to configure the camera
uint32_t line = 0;
uint32_t line_Frame = 0;
uint32_t vsync = 0;

// Configuration mode 
uint8_t OV7675_QQVGA[] =
{	
	 //MSB for image size, LSBs are equal to 0 !
	OV7675_HSTART, 
	0x2F, 
	OV7675_HSTOP,
	0x43,
	OV7675_VSTART, 
	0x5D,
	OV7675_VSTOP,
	OV7675_VSTOP_DV, // VSTOP can't be modified !
};

uint8_t OV7675_QVGA[] =
{	
};

uint8_t OV7675_VGA[] =
{	
};


/**
  * @brief  Initializes the hardware resources (I2C and GPIO) used to configure 
  *         the OV9655 camera.
  * @param  None
  * @retval None
  */
void OV7675_Init(OV7675_ModeTypeDef mode) 
{
	uint8_t pData = 0x7b;
	
	uint8_t pDataRead;
	uint32_t data_remaining;
	uint32_t capture;
	
	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1); // Main Clock Output 8Mhz to PA8
	
	// I2C
	OV7675_I2C_config();
	
	// Camera ID
	I2C_Status = OV7675_ReadID(&OV7675_Camera_ID);
	I2C_Status_Printf("ID", I2C_Status);
	
	// Reset all register. Warning return I2C_READ_FAILED because this register can't be read just after a writting !
	I2C_Status = OV7675_Write_Reg_Bit(OV7675_COM7, 7, 1);
	I2C_Status_Printf("Reset register", I2C_Status);
	
	// Change outpout format mode.
	switch (mode)
	{
	case QQVGA :
		I2C_Status = OV7675_Mode_Config(OV7675_QQVGA, sizeof(OV7675_QQVGA));
		I2C_Status_Printf("mode QQVGA", I2C_Status);
		break;
		
	case QVGA:
		I2C_Status = OV7675_Mode_Config(OV7675_QVGA, sizeof(OV7675_QVGA));
		I2C_Status_Printf("mode QVGA", I2C_Status);
		break;
		
	case VGA:
		I2C_Status = OV7675_Mode_Config(OV7675_VGA, sizeof(OV7675_VGA));
		I2C_Status_Printf("mode VGA", I2C_Status);
		break;
	}
	
	// DCMI
	OV7675_Dcmi_Config();
	
	printf("End of INIT\r\n");
}


/**
  * @brief  Send image through serial port thanks to printf
  * @param data : buffer with the value of the pixels
  * @param data_size : size of the buffer
  * @retval None
  */
void OV7675_Send_Data(uint8_t data[], uint32_t data_size)
{
	printf("START PBUFFER TRANSFER\r\n");
	for (int i = 0; i < data_size; i++)
	{
		printf("%d\r\n", data[i]);
	}
	printf("END PBUFFER TRANSFER\r\n");
}

/**
  * @brief  Initialize the DCMI and DMA transfer
  * @param  None
  * @retval None
  */
void OV7675_Dcmi_Config(void)
{	
	DCMI_Handle.Instance = DCMI;
	
	// DCMI Structure
	DCMI_Handle.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
	DCMI_Handle.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
	DCMI_Handle.Init.VSPolarity = DCMI_VSPOLARITY_HIGH; // Capture active on Vsync Low (logic inverted !)
	DCMI_Handle.Init.HSPolarity = DCMI_HSPOLARITY_LOW; // Capture active on Hsync High (logic inverted !)
	DCMI_Handle.Init.CaptureRate = DCMI_CR_ALL_FRAME;
	DCMI_Handle.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
	
	HAL_Status = HAL_DCMI_Init(&DCMI_Handle); 
	HAL_Status_Printf("DCMI Init", HAL_Status);
	
	DCMI_Handle.DMA_Handle = &DMA_Handle; // Put the adress of the DMA_Handle into the DCMI structure
	DCMI_Handle.DMA_Handle->Instance = DMA2_Stream1; // Base address for DMA2, it's also here that we specified the stream !
	DCMI_Handle.DMA_Handle->Parent = &DCMI_Handle;
	
	// DMA Struture
	DCMI_Handle.DMA_Handle->Init.Channel = DMA_CHANNEL_1;
	DCMI_Handle.DMA_Handle->Init.Direction = DMA_PERIPH_TO_MEMORY;
	DCMI_Handle.DMA_Handle->Init.PeriphInc = DMA_PINC_DISABLE;
	DCMI_Handle.DMA_Handle->Init.MemInc = DMA_MINC_ENABLE;
	DCMI_Handle.DMA_Handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	DCMI_Handle.DMA_Handle->Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	DCMI_Handle.DMA_Handle->Init.Mode = DMA_CIRCULAR;
	DCMI_Handle.DMA_Handle->Init.Priority = DMA_PRIORITY_HIGH;
	DCMI_Handle.DMA_Handle->Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	DCMI_Handle.DMA_Handle->Init.FIFOThreshold = DMA_FIFO_THRESHOLD_3QUARTERSFULL;
	DCMI_Handle.DMA_Handle->Init.MemBurst = DMA_MBURST_SINGLE;
	DCMI_Handle.DMA_Handle->Init.PeriphBurst = DMA_PBURST_SINGLE;
	
	HAL_Status = HAL_DMA_Init(DCMI_Handle.DMA_Handle);
	HAL_Status_Printf("DMA Init", HAL_Status);
	
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
	HAL_NVIC_SetPriority(DCMI_IRQn, 1, 2); // priority 0 and sub priority 0 (0 Highest priority)
	HAL_NVIC_EnableIRQ(DCMI_IRQn);
	
	HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 1, 1); // priority 0 and sub priority 0 (0 Highest priority)
	HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
	
	HAL_Status = HAL_DCMI_Start_DMA(&DCMI_Handle, DCMI_MODE_SNAPSHOT, (uint32_t) data, data_size / 4); // DCMI_MODE_SNAPSHOT
	HAL_Status_Printf("DCMI Start DMA", HAL_Status);
}

/**
  * @brief  Initialize the I2C1
  * @param  None
  * @retval None
  */
void OV7675_I2C_config(void)
{
	I2C_Handle.Instance = I2C1; // Allow HAL_I2C_Init to fill the wright registers (absolute value in stm32f407xx.h) !

	// I2C Structure
	I2C_Handle.Init.ClockSpeed = I2C_CLOCK_SPEED; // In OV7675_dcmi.h
	I2C_Handle.Init.DutyCycle = I2C_DUTYCYCLE_2;
	I2C_Handle.Init.OwnAddress1 = I2C_OWN_ADRESS; // In OV7675_dcmi.h
	I2C_Handle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	I2C_Handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	HAL_I2C_Init(&I2C_Handle);
}

/**
  * @brief  Configure the registers of the OV7675 camera according to the selected mode
  * @param  mode : uint8_t table with the adress and the value to put into registers
  * @retval I2C_StatusTypeDef
  */
I2C_StatusTypeDef OV7675_Mode_Config(uint8_t *mode, uint8_t size)
{
	uint32_t i;
	for (i = 0; i < size / 2; i++) 
	{
		I2C_Status = OV7675_Write_Reg(mode[2*i], &mode[2*i + 1]);
		if (I2C_Status != I2C_OK)
		{
			printf("Register failed : %#2x\r\n", mode[2*i]);
			printf("Value send : %#2x\r\n", mode[2*i + 1]);
			//return I2C_Status;
		}
	}
	return I2C_OK;
}

/**
  * @brief  Reads the OV9655 Manufacturer identifier.
  * @param  OV9655ID: pointer to the OV9655 Manufacturer identifier.
  * @retval None
  */
I2C_StatusTypeDef OV7675_ReadID(OV7675_IDTypeDef* OV7675ID)
{
	uint8_t pDataRead;
	
	OV7675_Read_Reg(OV7675_MIDH, &pDataRead);
	OV7675ID->Manufacturer_ID1 = pDataRead;
	if (OV7675ID->Manufacturer_ID1 != OV7675_MIDH_DV)
		return I2C_WRONG_VALUE;
	
	OV7675_Read_Reg(OV7675_MIDL, &pDataRead);
	OV7675ID->Manufacturer_ID2 = pDataRead;
	if (OV7675ID->Manufacturer_ID2 != OV7675_MIDL_DV)
		return I2C_WRONG_VALUE;
	
	OV7675_Read_Reg(OV7675_VER, &pDataRead);
	OV7675ID->Version = pDataRead;
	if (OV7675ID->Version != OV7675_VER_DV)
		return I2C_WRONG_VALUE;
	
	OV7675_Read_Reg(OV7675_PID, &pDataRead);
	OV7675ID->PID = pDataRead;
	if (OV7675ID->PID != OV7675_PID_DV)
		return I2C_WRONG_VALUE;
	
	return I2C_OK;
}

/**
  * @brief  Write a value into the camera OV7675 at a specific adress 
  * @param MemAdress : interna memory address where the value is written 
  * @param *pData : pointer on the data sended
  * @retval I2C_StatusTypeDef
  */
I2C_StatusTypeDef OV7675_Write_Reg(uint16_t MemAddress, uint8_t *pData)
{
	uint8_t pDataRead;
	HAL_StatusTypeDef HAL_Status;
	
	HAL_Status = HAL_I2C_Mem_Write(&I2C_Handle, OV7675_DEVICE_WRITE_ADDRESS, MemAddress, I2C_MEMADD_SIZE_8BIT, pData, 1, DCMI_TIMEOUT_MAX);
	if (HAL_Status != HAL_OK)
		return I2C_WRITE_FAILED;
	
	HAL_Status = HAL_I2C_Mem_Read(&I2C_Handle, OV7675_DEVICE_READ_ADDRESS, MemAddress, I2C_MEMADD_SIZE_8BIT, &pDataRead, 1, DCMI_TIMEOUT_MAX);
	if (HAL_Status != HAL_OK)
		return I2C_READ_FAILED;
	
	if (*pData == pDataRead)
		return I2C_OK;
	else
		return I2C_WRONG_VALUE;
		
}

/**
  * @brief  Write a specific bit into a register of the OV7675 camera 
  * @param MemAdress : internal memory address where the value is written 
  * @param bit : represents the bit that needs to be changed. Warning the first bit is 0 !
  * @param value : the bit will have this value at the end of the function. =0 or =1 only !
  * @retval I2C_StatusTypeDef
  */
I2C_StatusTypeDef OV7675_Write_Reg_Bit(uint16_t MemAddress, uint8_t bit, uint8_t value)
{
	uint8_t data = 0;
	uint8_t dataRead_before;
	uint8_t dataRead_after;
	
	HAL_Status = HAL_I2C_Mem_Read(&I2C_Handle, OV7675_DEVICE_READ_ADDRESS, MemAddress, I2C_MEMADD_SIZE_8BIT, &dataRead_before, 1, DCMI_TIMEOUT_MAX);
	if (HAL_Status != HAL_OK)
		return I2C_READ_FAILED;
		
	if (value == 1)
	{
		data = 1 << bit;
		data |= dataRead_before;
	}
	else if (value == 0)
	{
		data = ~(1 << bit);
		data = data & dataRead_before;
	}
	else
	{
		return I2C_WRONG_VALUE;
	}
	
	printf("data sent : %#2x\r\n", data);
	
	HAL_Status = HAL_I2C_Mem_Write(&I2C_Handle, OV7675_DEVICE_WRITE_ADDRESS, MemAddress, I2C_MEMADD_SIZE_8BIT, &data, 1, DCMI_TIMEOUT_MAX);
	if (HAL_Status != HAL_OK)
		return I2C_WRITE_FAILED;
	
	HAL_Status = HAL_I2C_Mem_Read(&I2C_Handle, OV7675_DEVICE_READ_ADDRESS, MemAddress, I2C_MEMADD_SIZE_8BIT, &dataRead_after, 1, DCMI_TIMEOUT_MAX);
	if (HAL_Status != HAL_OK)
		return I2C_READ_FAILED;
	
	if (data == dataRead_after)
		return I2C_OK;
	else
	{
		printf("Wrong register data : %#2x\r\n", dataRead_after);
		return I2C_WRONG_VALUE;
	}
}
/**
  * @brief  Read a value into the camera OV7675 at a specific adress 
  * @param MemAdress : interna memory address where the value is read 
  * @param *pDataRead : pointer on the received data
  * @retval I2C_StatusTypeDef
  */
I2C_StatusTypeDef OV7675_Read_Reg(uint16_t MemAddress, uint8_t *pDataRead)
{
	HAL_StatusTypeDef HAL_Status;
	HAL_Status = HAL_I2C_Mem_Read(&I2C_Handle, OV7675_DEVICE_READ_ADDRESS, MemAddress, I2C_MEMADD_SIZE_8BIT, pDataRead, 1, DCMI_TIMEOUT_MAX);
	
	if (HAL_Status != HAL_OK)
		return I2C_READ_FAILED;
	else
		return I2C_OK;
}

/**
  * Printf function for HAL_StatusTypeDef with a little message before
  */
void HAL_Status_Printf(const char before[], HAL_StatusTypeDef HStatus)
{
	printf("HAL: ");
	printf("%s", before);	
	printf(" : ");
	
	if (HStatus == HAL_OK)
		printf("Ok");
	if (HStatus == HAL_ERROR)
		printf("Error");
	if (HStatus == HAL_BUSY)
		printf("Busy");
	if (HStatus == HAL_TIMEOUT)
		printf("Timeout");
	
	printf("\r\n");
}

/**
  * Printf function for I2C_StatusTypeDef with a little message before
  */
void I2C_Status_Printf(const char before[], I2C_StatusTypeDef HStatus)
{
	printf("I2C: ");
	printf("%s", before);	
	printf(": ");
	
	if (HStatus == I2C_OK)
		printf("Ok");
	if (HStatus == I2C_WRONG_VALUE)
		printf("Wrong value");
	if (HStatus == I2C_WRITE_FAILED)
		printf("Write failed");
	if (HStatus == I2C_READ_FAILED)
		printf("Read failed");
	
	printf("\r\n");
}

/**
  * @brief  Initializes the I2C MSP.
  * @param  hdcmi: pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for I2C.
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	__HAL_RCC_I2C1_CLK_ENABLE(); // Clock I2C1
	__HAL_RCC_GPIOB_CLK_ENABLE(); // Clock port B enable

	GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStructure.Alternate = GPIO_AF4_I2C1; // The number of the I2Cx doesn't change the value (0x04). 
													//The number is just for different pin, automatic connection ?
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
  * @brief  Initializes the DCMI MSP.
  * @param  hdcmi: pointer to a DCMI_HandleTypeDef structure that contains
  *                the configuration information for DCMI.
  * @retval None
  */
void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	__HAL_RCC_DCMI_CLK_ENABLE(); // Clock DCMI
	__HAL_RCC_DMA2_CLK_ENABLE(); // Clock DMA2
	
	// D0 (PC6), D1 (PC7), D2 (PC8), D3 (PC9), D4 (PC11)
	__HAL_RCC_GPIOC_CLK_ENABLE(); // Clock port C enable
	GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_11;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStructure.Alternate = GPIO_AF13_DCMI; 
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// D5 (PB6), Vsync (PB7)
	__HAL_RCC_GPIOB_CLK_ENABLE(); // Clock port B enable
	GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// D6 (PE5), D7 (PE6)
	__HAL_RCC_GPIOE_CLK_ENABLE(); // Clock port B enable
	GPIO_InitStructure.Pin = GPIO_PIN_5 | GPIO_PIN_6;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	// Hsync (PA4), Pxclk (PA6)
	__HAL_RCC_GPIOA_CLK_ENABLE(); // Clock port B enable
	GPIO_InitStructure.Pin = GPIO_PIN_4 | GPIO_PIN_6;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}

// Warning printf in callback fct creates a failed in the software
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	line_Frame = line;
	line = 0;
	vsync = 0;
	//asm("bkpt 255");
}

void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{
	//asm("bkpt 255");
}

void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	if (vsync == 1)
	{
		line++;
		//counter[line] = __HAL_DMA_GET_COUNTER(hdcmi->DMA_Handle);
	}
}

void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	vsync = 1;
}

extern "C" void DCMI_IRQHandler(void)   
{
	HAL_DCMI_IRQHandler(&DCMI_Handle);
}

extern "C" void DMA2_Stream1_IRQHandler()
{
	HAL_DMA_IRQHandler(DCMI_Handle.DMA_Handle);
}














