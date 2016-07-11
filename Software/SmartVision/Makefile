
# Put your stlink folder here so make burn will work.
# STLINK=~/stlink.git

# Put your STM32F4 library code directory here
STM_COMMON=../STM32F4-Discovery_FW_V1.1.0

# Put your source files here (or *.c, etc)
SRCS = main.c system_stm32f4xx.c stm32f4xx_it.c 

# Personal library
SRCS += dcmi_ov7675.c
SRCS += camera_api.c

# Add .c files from library
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/misc.c
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.c
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c

	# USB SRCS
SRCS += $(STM_COMMON)/../SmartVision/usb_lib/cdc/usbd_cdc_vcp.c
SRCS += $(STM_COMMON)/../SmartVision/usb_lib/core/usbd_core.c
SRCS += $(STM_COMMON)/../SmartVision/usb_conf/usbd_usr.c
SRCS += $(STM_COMMON)/../SmartVision/usb_conf/usbd_desc.c
SRCS += $(STM_COMMON)/../SmartVision/usb_lib/cdc/usbd_cdc_core.c
SRCS += $(STM_COMMON)/../SmartVision/usb_lib/otg/usb_dcd_int.c
SRCS += $(STM_COMMON)/../SmartVision/usb_lib/otg/usb_core.c
SRCS += $(STM_COMMON)/../SmartVision/usb_conf/usb_bsp.c
SRCS += $(STM_COMMON)/../SmartVision/usb_lib/otg/usb_dcd.c
SRCS += $(STM_COMMON)/../SmartVision/usb_lib/core/usbd_req.c
SRCS += $(STM_COMMON)/../SmartVision/usb_lib/core/usbd_ioreq.c




# SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c
# SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c



# SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c

# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=SmartVision



# Normally you shouldn't need to change anything below this line!
#######################################################################################

CC=../gcc-arm-none-eabi-4_8-2014q1/bin/arm-none-eabi-gcc
OBJCOPY=../gcc-arm-none-eabi-4_8-2014q1/bin/arm-none-eabi-objcopy

# With optimization
# CFLAGS  = -g -O2 -Wall -Tstm32_flash.ld 
# Without optimization
CFLAGS  = -g -O0 -Wall -Tstm32_flash.ld

CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -I.

# Include files from STM libraries
CFLAGS += -I$(STM_COMMON)/Utilities/STM32F4-Discovery
CFLAGS += -I$(STM_COMMON)/Libraries/CMSIS/Include 
CFLAGS += -I$(STM_COMMON)/Libraries/CMSIS/ST/STM32F4xx/Include
CFLAGS += -I$(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/inc/
		# USB library 
CFLAGS += -I$(STM_COMMON)/../SmartVision/usb_lib/cdc
CFLAGS += -I$(STM_COMMON)/../SmartVision/usb_lib/core
CFLAGS += -I$(STM_COMMON)/../SmartVision/usb_lib/otg
CFLAGS += -I$(STM_COMMON)/../SmartVision/usb_conf


# add startup file to build
SRCS += $(STM_COMMON)/Libraries/CMSIS/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f4xx.s 
OBJS = $(SRCS:.c=.o)


.PHONY: proj

all: proj

proj: $(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ 
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin


clean:
	rm -f *.o $(PROJ_NAME).elf $(PROJ_NAME).hex $(PROJ_NAME).bin

# # Flash the STM32F4
# burn: proj
# 	$(STLINK)/st-flash write $(PROJ_NAME).bin 0x8000000
