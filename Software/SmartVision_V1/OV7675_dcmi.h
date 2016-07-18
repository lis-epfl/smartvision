#include <stm32f4xx_hal_rcc.h>
#include <stm32f4xx_hal_gpio.h>
#include <stm32f4xx_hal_i2c.h>
#include <stm32f4xx_hal_dma.h>
#include <stm32f4xx_hal_dcmi.h>

#include <stm32f4xx_hal.h> // To handle SYSTick
#include <stm32f4xx_hal_cortex.h> // NVIC to handle interrupt

#include <stdio.h>

//Personal Library
#include <OV7675_registers.h>

// Printf define
#define OV7675_DCMI_PRINTF 1

// I2C defines
#define I2C_CLOCK_SPEED 100000
#define I2C_OWN_ADRESS 0xFE;
#define OV7675_DEVICE_WRITE_ADDRESS 0x42
#define OV7675_DEVICE_READ_ADDRESS 0x43
#define DCMI_TIMEOUT_MAX  10 // 10000 for 168MHz

// Image size
#define QQVGA_YUV_size 38400 // QQVGA : 160(column)*120(line) pixels = 19200 , *2 if YUV422 is selected

// Return status
typedef enum 
{
	I2C_OK           = 0x00,
	I2C_WRONG_VALUE  = 0x01,
	I2C_WRITE_FAILED = 0x02,
	I2C_READ_FAILED  = 0x03,
} I2C_StatusTypeDef;

// OV7675 Structure
typedef struct
{
	uint8_t Manufacturer_ID1;
	uint8_t Manufacturer_ID2;
	uint8_t Version;
	uint8_t PID;
}OV7675_IDTypeDef;

// OV7675 Output format mode
typedef enum 
{
	QQVGA = 0x00,
	QVGA  = 0x01,
	VGA   = 0x02,
} OV7675_ModeTypeDef;


void OV7675_Init(OV7675_ModeTypeDef mode); 

// I2C
void OV7675_I2C_config();
I2C_StatusTypeDef OV7675_Write_Reg(uint16_t MemAddress, uint8_t *pData);
I2C_StatusTypeDef OV7675_Write_Reg_Bit(uint16_t MemAddress, uint8_t bit, uint8_t value);
I2C_StatusTypeDef OV7675_Read_Reg(uint16_t MemAddress, uint8_t *pDataRead);
I2C_StatusTypeDef OV7675_ReadID(OV7675_IDTypeDef* OV7675ID);
I2C_StatusTypeDef OV7675_Mode_Config(uint8_t *mode, uint8_t size);

// DCMI
void OV7675_Dcmi_Config(void);

//Output Serial Port function
void HAL_Status_Printf(const char before[], HAL_StatusTypeDef HStatus);
void I2C_Status_Printf(const char before[], I2C_StatusTypeDef HStatus);
void OV7675_Send_Data(uint8_t data[], uint32_t data_size);


