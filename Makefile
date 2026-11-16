TARGET := firmware
CUBE := HAL/STM32CubeF1
BUILD_DIR := build
HAL := $(CUBE)/Drivers/STM32F1xx_HAL_Driver
CMSIS := $(CUBE)/Drivers/CMSIS
FREERTOS := RTOS/FreeRTOS-Kernel
BOOTLOADER := bootloader/dapboot

CC := arm-none-eabi-gcc
AS := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE := arm-none-eabi-size

CPU := cortex-m3
CPUFLAGS := -mcpu=$(CPU) -mthumb -mfloat-abi=soft 
DEFINES := -DSTM32F103xB -DUSE_HAL_DRIVER 
CFLAGS := $(CPUFLAGS) -std=c11 -Wall -Wextra -O0 -g3 \
	-ffunction-sections -fdata-sections $(DEFINES)\
	-MMD -MP \

ASFLAGS = -mcpu=$(CPU) -mthumb -mfloat-abi=soft -g3

LDSCRIPT   := linker/stm32f103c8tx_flash.ld
FLASH_ADDR := 0x08000000

LDFLAGS = $(CPUFLAGS)\
	-T$(LDSCRIPT) \
	-Wl,--gc-sections \
	-Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
	-specs=nosys.specs -specs=nano.specs 

INCLUDES = -ICore/inc \
	-I$(HAL)/Inc \
	-I$(CMSIS)/Include \
	-I$(CMSIS)/Device/ST/STM32F1xx/Include 

STARTUP := $(CMSIS)/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xb.s
SYSTEM := $(CMSIS)/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c

FREERTOS_FILES := $(FREERTOS)/portable/GCC/ARM_CM3/port.c \
	$(FREERTOS)/portable/MemMang/heap_4.c \
	$(FREERTOS)/tasks.c \
	$(FREERTOS)/list.c \
	$(FREERTOS)/queue.c 

C_SOURCES = $(SYSTEM)\
	Core/src/main.c \
	Core/src/stm32f1xx_it.c \
	$(HAL)/Src/stm32f1xx_hal.c \
	$(HAL)/Src/stm32f1xx_hal_cortex.c \
	$(HAL)/Src/stm32f1xx_hal_gpio.c \
	$(HAL)/Src/stm32f1xx_hal_rcc.c \
	#$(HAL)/Src/stm32f1xx_hal_flash.c \
	#$(HAL)/Src/stm32f1xx_hal_dma.c \
	#$(HAL)/Src/stm32f1xx_hal_tim.c \
	#$(HAL)/Src/stm32f1xx_hal_uart.c \
	#$(HAL)/Src/stm32f1xx_hal_tim_ex.c
  
C_SOURCES += $(FREERTOS_FILES)

CFLAGS += -I$(FREERTOS)/include
CFLAGS += -I$(FREERTOS)/portable/GCC/ARM_CM3

C_OBJECTS   = $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS = $(patsubst %.s,$(BUILD_DIR)/%.o,$(STARTUP))
OBJECTS     = $(C_OBJECTS) $(ASM_OBJECTS)

all: $(BUILD_DIR)/$(TARGET).elf \
     $(BUILD_DIR)/$(TARGET).bin \
     $(BUILD_DIR)/$(TARGET).hex

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

flash: $(BUILD_DIR)/$(TARGET).bin
	st-flash write $< $(FLASH_ADDR)

-include $(OBJECTS:.o=.d)

.PHONY: all clean flash
