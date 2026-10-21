# Dùng chung cho hai firmware; PROJECT_DIR được định nghĩa ở Makefile của bài.
.DEFAULT_GOAL := all
COMMON_DIR := $(abspath $(PROJECT_DIR)/../FreeRTOS_Common)
CUBE       := $(abspath $(PROJECT_DIR)/../STM32CubeF1)
CMSIS      := $(CUBE)/Drivers/CMSIS
DEVICE     := $(CMSIS)/Device/ST/STM32F1xx
RTOS       := $(CUBE)/Middlewares/Third_Party/FreeRTOS/Source
BUILD      := $(PROJECT_DIR)/build
TARGET     := firmware
PREFIX     ?= arm-none-eabi-
CC         := $(PREFIX)gcc
OBJCOPY    := $(PREFIX)objcopy
SIZE       := $(PREFIX)size

INCLUDES := -I$(PROJECT_DIR)/inc -I$(COMMON_DIR)/inc \
            -I$(CMSIS)/Include -I$(DEVICE)/Include \
            -I$(RTOS)/include -I$(RTOS)/portable/GCC/ARM_CM3
CPU      := -mcpu=cortex-m3 -mthumb
CFLAGS   := $(CPU) -DSTM32F103xB $(INCLUDES) -std=c11 -O2 -g3 \
            -Wall -Wextra -ffunction-sections -fdata-sections -MMD -MP
LDFLAGS  := $(CPU) -T$(COMMON_DIR)/STM32F103C8.ld \
            -Wl,--gc-sections -Wl,-Map=$(BUILD)/$(TARGET).map \
            -specs=nano.specs -specs=nosys.specs

APP_NAMES := $(notdir $(wildcard $(PROJECT_DIR)/src/*.c))
APP_OBJS  := $(addprefix $(BUILD)/app_,$(APP_NAMES:.c=.o))
COMMON_NAMES := board gpio rtos_hooks
COMMON_OBJS := $(addprefix $(BUILD)/common_,$(addsuffix .o,$(COMMON_NAMES)))
RTOS_NAMES  := tasks queue list timers
RTOS_OBJS   := $(addprefix $(BUILD)/rtos_,$(addsuffix .o,$(RTOS_NAMES)))
OBJS := $(APP_OBJS) $(COMMON_OBJS) $(RTOS_OBJS) \
        $(BUILD)/port.o $(BUILD)/heap_4.o $(BUILD)/system.o $(BUILD)/startup.o

.PHONY: all flash clean test
all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).bin

$(BUILD):
	mkdir -p "$@"

$(BUILD)/app_%.o: $(PROJECT_DIR)/src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/common_%.o: $(COMMON_DIR)/src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/rtos_%.o: $(RTOS)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/port.o: $(RTOS)/portable/GCC/ARM_CM3/port.c | $(BUILD)
	$(CC) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/heap_4.o: $(RTOS)/portable/MemMang/heap_4.c | $(BUILD)
	$(CC) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/system.o: $(DEVICE)/Source/Templates/system_stm32f1xx.c | $(BUILD)
	$(CC) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/startup.o: $(DEVICE)/Source/Templates/gcc/startup_stm32f103xb.s | $(BUILD)
	$(CC) $(CPU) -g3 -c "$<" -o "$@"

$(BUILD)/$(TARGET).elf: $(OBJS) $(COMMON_DIR)/STM32F103C8.ld
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"
	$(SIZE) "$@"

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary "$<" "$@"

OPENOCD_ROOT    ?= D:/Downloads/xpack-openocd-0.12.0-7-win32-x64/xpack-openocd-0.12.0-7
OPENOCD         ?= $(OPENOCD_ROOT)/bin/openocd.exe
OPENOCD_SCRIPTS ?= $(OPENOCD_ROOT)/openocd/scripts
OPENOCD_IMAGE   = $(shell cygpath -m "$(BUILD)/$(TARGET).elf" 2>/dev/null || echo "$(BUILD)/$(TARGET).elf")

flash: all
	"$(OPENOCD)" -s "$(OPENOCD_SCRIPTS)" -f interface/stlink.cfg \
	    -c "transport select swd" -f target/stm32f1x.cfg \
	    -c "program {$(OPENOCD_IMAGE)} verify reset exit"

# Chỉ xóa artifact trong build của đúng bài, không xóa cây project.
clean:
	rm -f "$(BUILD)"/*.o "$(BUILD)"/*.d "$(BUILD)"/*.elf "$(BUILD)"/*.bin "$(BUILD)"/*.map

-include $(OBJS:.o=.d)

# Test logic driver trên PC với thanh ghi/API giả (không nạp STM32).
HOST_CC ?= gcc
TEST_DIR := $(COMMON_DIR)/tests
TEST_BUILD := $(TEST_DIR)/build
TEST_EXE := $(TEST_BUILD)/test_drivers.exe
TEST_SOURCES := $(TEST_DIR)/test_drivers.c \
                $(COMMON_DIR)/../BT_1_FreeRTOS/src/led_control.c \
                $(COMMON_DIR)/../BT_1_FreeRTOS/src/led_blink.c \
                $(COMMON_DIR)/../BT_1_FreeRTOS/src/button.c \
                $(COMMON_DIR)/../BT_1_FreeRTOS/src/led_pwm.c \
                $(COMMON_DIR)/../BT_2_FreeRTOS/src/button_exti.c
TEST_HEADERS := $(wildcard $(TEST_DIR)/fakes/*.h) \
                $(wildcard $(COMMON_DIR)/../BT_1_FreeRTOS/inc/*.h) \
                $(wildcard $(COMMON_DIR)/../BT_2_FreeRTOS/inc/*.h)

$(TEST_BUILD):
	mkdir -p "$@"

$(TEST_EXE): $(TEST_SOURCES) $(TEST_HEADERS) | $(TEST_BUILD)
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror \
	    -I$(TEST_DIR)/fakes -I$(COMMON_DIR)/../BT_1_FreeRTOS/inc \
	    -I$(COMMON_DIR)/../BT_2_FreeRTOS/inc $(TEST_SOURCES) -o "$@"

test: $(TEST_EXE)
	"$(TEST_EXE)"
