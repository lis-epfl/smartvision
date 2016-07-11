/**
  ******************************************************************************
  * @file    main.c
  * @author  William PONSOT
  * @version V1.0.0
  * @date    15 mars 2016
  * @brief   main file of SmartVision project
  ******************************************************************************
*/

#define HSE_VALUE ((uint32_t)8000000) /* STM32 discovery uses a 8Mhz external crystal */

// Configration header
#include "stm32f4xx_conf.h"
#include <stm32f4xx.h>

// STM library
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_dcmi.h>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_it.h> 
#include <stm32f4xx_exti.h> //for control of external interrupt (USB) 


// Personal library
#include "dcmi_ov7675.h" 
#include "camera_api.h" 

// USB library
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "usb_dcd_int.h"


volatile uint32_t ticker, downTicker;


/*
 * The USB data must be 4 byte aligned if DMA is enabled. This macro handles
 * the alignment, if necessary (it's actually magic, but don't tell anyone).
 */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;


void init();
void ColorfulRingOfDeath(void);

/*
 * Define prototypes for interrupt handlers here. The conditional "extern"
 * ensures the weak declarations from startup_stm32f4xx.c are overridden.
 */
#ifdef __cplusplus
 extern "C" {
#endif

void SysTick_Handler(void);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void OTG_FS_IRQHandler(void);
void OTG_FS_WKUP_IRQHandler(void);

#ifdef __cplusplus
}
#endif



RCC_ClocksTypeDef RCC_Clocks;
OV7675_IDTypeDef  OV7675_Camera_ID;  


//Quick hack, approximately 1ms delay
void ms_delay(int ms)
{
   while (ms-- > 0) {
      volatile int x=5971;
      while (x-- > 0)
         __asm("nop");
   }
}


int main(void)
{
  uint8_t theByte;
  
  /* Set up the system clocks */
  SystemInit();

  /* Initialize USB, IO, SysTick, and all those other things you do in the morning */
  init();


  while (1)
  {
    /* Blink the orange LED at 1Hz */
    if (500 == ticker)
    {
      GPIOD->BSRRH = GPIO_Pin_13;
    }
    else if (1000 == ticker)
    {
      ticker = 0;
      GPIOD->BSRRL = GPIO_Pin_13;
    }

    /* If there's data on the virtual serial port:
     *  - Echo it back
     *  - Turn the green LED on for 10ms
     */
    
    if (VCP_get_char(&theByte))
    {
      VCP_put_char(theByte);


      GPIOD->BSRRL = GPIO_Pin_12;
      downTicker = 10;
    }
    if (0 == downTicker)
    {
      GPIOD->BSRRH = GPIO_Pin_12;
    }
  }

  return 0;  
}

void init()
{
  /* STM32F4 discovery LEDs */
  GPIO_InitTypeDef LED_Config;

  /* Always remember to turn on the peripheral clock...  If not, you may be up till 3am debugging... */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  LED_Config.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
  LED_Config.GPIO_Mode = GPIO_Mode_OUT;
  LED_Config.GPIO_OType = GPIO_OType_PP;
  LED_Config.GPIO_Speed = GPIO_Speed_25MHz;
  LED_Config.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &LED_Config);



  /* Setup SysTick or CROD! */
  if (SysTick_Config(SystemCoreClock / 1000))
  {
    ColorfulRingOfDeath();
  }


  /* Setup USB */
  USBD_Init(&USB_OTG_dev,
              USB_OTG_FS_CORE_ID,
              &USR_desc,
              &USBD_CDC_cb,
              &USR_cb);

  return;
}

void ColorfulRingOfDeath(void)
{
  uint16_t ring = 1;
  while (1)
  {
    uint32_t count = 0;
    while (count++ < 500000);

    GPIOD->BSRRH = (ring << 12);
    ring = ring << 1;
    if (ring >= 1<<4)
    {
      ring = 1;
    }
    GPIOD->BSRRL = (ring << 12);
  }
}

/*
 * Interrupt Handlers
 */

void SysTick_Handler(void)
{
  ticker++;
  if (downTicker > 0)
  {
    downTicker--;
  }
}

void NMI_Handler(void)       {}
void HardFault_Handler(void) { ColorfulRingOfDeath(); }
void MemManage_Handler(void) { ColorfulRingOfDeath(); }
void BusFault_Handler(void)  { ColorfulRingOfDeath(); }
void UsageFault_Handler(void){ ColorfulRingOfDeath(); }
void SVC_Handler(void)       {}
void DebugMon_Handler(void)  {}
void PendSV_Handler(void)    {}

void OTG_FS_IRQHandler(void)
{
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

void OTG_FS_WKUP_IRQHandler(void)
{
  if(USB_OTG_dev.cfg.low_power)
  {
    *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
    SystemInit();
    USB_OTG_UngateClock(&USB_OTG_dev);
  }
  EXTI_ClearITPendingBit(EXTI_Line18);
}


// !!! KEEP IT !!!  OLD CODE for camera interface

  // uint8_t i = 0;
  // uint16_t bcounter;
  // /* TypeDef */
  // GPIO_InitTypeDef GPIO_InitStructure;

  // /* SysTick end of count event each 10ms */
  // RCC_GetClocksFreq(&RCC_Clocks);
  // SysTick_Config(RCC_Clocks.HCLK_Frequency / 100); // Usefull to manage Delay with Systick interrupt

// // PA8 --> Main Clock Output (A mettre plus tard dans OV7675_HW_Init avec les autres init )
  // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // Enable the clock on port A
  // RCC_MCO1Config(RCC_MCO1Source_HSE, RCC_MCO1Div_1); // Set of the MCO clock and prescaler
  // GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO); // PA8 --> MCO
  // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  // GPIO_Init(GPIOA, &GPIO_InitStructure);

  // OV7675_HW_Init();
  // OV7675_ReadID(&OV7675_Camera_ID); // Get information from the camera by I2C
  // OV7675_Init(BMP_QQVGA);
  // OV7675_QQVGAConfig(); // Send information to configure the canera for QQVGA (a lot of register are modified)

  // /* Enable DMA2 stream 1 and DCMI interface then start image capture */
  // DMA_Cmd(DMA2_Stream1, ENABLE); 
  // DCMI_Cmd(ENABLE); 

  // USB configuration

    

  



  // Delay(200);

 

  // DCMI_CaptureCmd(ENABLE);
  //  while(1)
  // {
  //   bcounter = DMA_GetCurrDataCounter(DMA2_Stream1);
  // }


// // Private typedef 
// GPIO_InitTypeDef  GPIO_InitStructure;

// // !< At this stage the microcontroller clock setting is already configured, 
// //        this is done through SystemInit() function which is called from startup
// //        file (startup_stm32f4xx.s) before to branch to application main.
// //        To reconfigure the default setting of SystemInit() function, refer to
// //         system_stm32f4xx.c file
     

//   GPIOD Periph clock enable 
//   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

//   // Configure PD12, PD13, PD14 and PD15 in output pushpull mode 
//   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
//   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//   GPIO_Init(GPIOD, &GPIO_InitStructure);


//   //User button
//   uint8_t button; //To save the output value of the button
//   // GPIOA Periph clock enable 
//   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
//   // Configure PA0 
//   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//   GPIO_Init(GPIOA, &GPIO_InitStructure);

//   while (1)
//   {
//     button = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0); //Read the value of the button

//     if (button==1)
//     {
//       GPIO_SetBits(GPIOD, GPIO_Pin_12); 
//       ms_delay(50);
//       GPIO_SetBits(GPIOD, GPIO_Pin_13);
//       ms_delay(50);
//       GPIO_SetBits(GPIOD, GPIO_Pin_14);
//       ms_delay(50);
//       GPIO_SetBits(GPIOD, GPIO_Pin_15);
//       ms_delay(50);

//       GPIO_ResetBits(GPIOD, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
//     }
//     else
//     {
//       GPIO_ResetBits(GPIOD, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
//     }  
//   }