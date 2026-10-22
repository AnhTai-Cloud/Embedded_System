# Dùng chung cho hai firmware; PROJECT_DIR được định nghĩa ở Makefile của bài.
# all là mục tiêu mặc định, kể cả khi các file dependency .d được include ở dưới.
.DEFAULT_GOAL := all
# Xác định thư mục theo vị trí project để make -C hoặc make -f đều dùng đúng source.
COMMON_DIR := $(abspath $(PROJECT_DIR)/../FreeRTOS_Common)
CUBE       := $(abspath $(PROJECT_DIR)/../STM32CubeF1)
CMSIS      := $(CUBE)/Drivers/CMSIS
DEVICE     := $(CMSIS)/Device/ST/STM32F1xx
RTOS       := $(CUBE)/Middlewares/Third_Party/FreeRTOS/Source
BUILD      := $(PROJECT_DIR)/build
TARGET     := firmware
# Có thể đổi PREFIX để chọn toolchain khác cùng họ arm-none-eabi.
PREFIX     ?= arm-none-eabi-
CC         := $(PREFIX)gcc
OBJCOPY    := $(PREFIX)objcopy
SIZE       := $(PREFIX)size

# -I chỉ nơi tìm header ứng dụng, phần dùng chung, CMSIS và port FreeRTOS ARM_CM3.
INCLUDES := -I$(PROJECT_DIR)/inc -I$(COMMON_DIR)/inc \
            -I$(CMSIS)/Include -I$(DEVICE)/Include \
            -I$(RTOS)/include -I$(RTOS)/portable/GCC/ARM_CM3
CPU      := -mcpu=cortex-m3 -mthumb
# Biên dịch C11, tối ưu O2, giữ thông tin debug và bật cảnh báo.
# -MMD/-MP sinh dependency header; section riêng cho từng hàm/biến giúp bỏ mã không dùng.
CFLAGS   := $(CPU) -DSTM32F103xB $(INCLUDES) -std=c11 -O2 -g3 \
            -Wall -Wextra -ffunction-sections -fdata-sections -MMD -MP
# -T chọn linker script; map ghi bố trí bộ nhớ; nano/nosys dùng runtime cho hệ nhúng.
LDFLAGS  := $(CPU) -T$(COMMON_DIR)/STM32F103C8.ld \
            -Wl,--gc-sections -Wl,-Map=$(BUILD)/$(TARGET).map \
            -specs=nano.specs -specs=nosys.specs

# Tự lấy các file C trong src/. Tiền tố app_/common_/rtos_ tránh trùng tên object.
APP_NAMES := $(notdir $(wildcard $(PROJECT_DIR)/src/*.c))
APP_OBJS  := $(addprefix $(BUILD)/app_,$(APP_NAMES:.c=.o))
COMMON_NAMES := board gpio rtos_hooks
COMMON_OBJS := $(addprefix $(BUILD)/common_,$(addsuffix .o,$(COMMON_NAMES)))
RTOS_NAMES  := tasks queue list timers
RTOS_OBJS   := $(addprefix $(BUILD)/rtos_,$(addsuffix .o,$(RTOS_NAMES)))
# port xử lý chuyển ngữ cảnh Cortex-M3; heap_4 cấp phát; startup/system đến từ CMSIS.
OBJS := $(APP_OBJS) $(COMMON_OBJS) $(RTOS_OBJS) \
        $(BUILD)/port.o $(BUILD)/heap_4.o $(BUILD)/system.o $(BUILD)/startup.o

.PHONY: all flash clean test
# Build ELF để debug/nạp và BIN chứa ảnh FLASH thuần.
all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).bin

$(BUILD):
	mkdir -p "$@"

# Các rule mẫu đổi từng file C thành object. Điều kiện sau dấu | bảo đảm
# thư mục build tồn tại mà không buộc biên dịch lại khi timestamp thư mục đổi.
# $< là source đầu tiên, $@ là tên output của rule đang chạy.
$(BUILD)/app_%.o: $(PROJECT_DIR)/src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/common_%.o: $(COMMON_DIR)/src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/rtos_%.o: $(RTOS)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c "$<" -o "$@"

# Chỉ chọn đúng port Cortex-M3 và bộ cấp phát heap_4, không build mọi port trong SDK.
$(BUILD)/port.o: $(RTOS)/portable/GCC/ARM_CM3/port.c | $(BUILD)
	$(CC) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/heap_4.o: $(RTOS)/portable/MemMang/heap_4.c | $(BUILD)
	$(CC) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/system.o: $(DEVICE)/Source/Templates/system_stm32f1xx.c | $(BUILD)
	$(CC) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/startup.o: $(DEVICE)/Source/Templates/gcc/startup_stm32f103xb.s | $(BUILD)
	$(CC) $(CPU) -g3 -c "$<" -o "$@"

# Liên kết toàn bộ object, dùng linker script để kiểm tra giới hạn FLASH/RAM.
$(BUILD)/$(TARGET).elf: $(OBJS) $(COMMON_DIR)/STM32F103C8.ld
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"
	$(SIZE) "$@"

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary "$<" "$@"

# Đường dẫn mặc định trên máy phát triển; có thể ghi đè OPENOCD/OPENOCD_SCRIPTS
# khi gọi make trên máy khác, không cần chỉnh source ứng dụng.
OPENOCD_ROOT    ?= D:/Downloads/xpack-openocd-0.12.0-7-win32-x64/xpack-openocd-0.12.0-7
OPENOCD         ?= $(OPENOCD_ROOT)/bin/openocd.exe
OPENOCD_SCRIPTS ?= $(OPENOCD_ROOT)/openocd/scripts
# OpenOCD native Windows cần dạng D:/... thay cho đường dẫn /d/... của MSYS.
OPENOCD_IMAGE   = $(shell cygpath -m "$(BUILD)/$(TARGET).elf" 2>/dev/null || echo "$(BUILD)/$(TARGET).elf")

# Chỉ nạp sau khi build thành công; verify kiểm tra lại FLASH, reset cho chạy chương trình.
flash: all
	"$(OPENOCD)" -s "$(OPENOCD_SCRIPTS)" -f interface/stlink.cfg \
	    -c "transport select swd" -f target/stm32f1x.cfg \
	    -c "program {$(OPENOCD_IMAGE)} verify reset exit"

# Chỉ xóa artifact trong build của đúng bài, không xóa cây project.
clean:
	rm -f "$(BUILD)"/*.o "$(BUILD)"/*.d "$(BUILD)"/*.elf "$(BUILD)"/*.bin "$(BUILD)"/*.map

# Dấu - cho phép bỏ qua file .d chưa tồn tại ở lần build đầu tiên.
-include $(OBJS:.o=.d)

# Test logic driver trên PC với thanh ghi/API giả (không nạp STM32).
# HOST_CC là GCC chạy trên PC, khác arm-none-eabi-gcc tạo mã cho Cortex-M3.
HOST_CC ?= gcc
TEST_DIR := $(COMMON_DIR)/tests
TEST_BUILD := $(TEST_DIR)/build
TEST_EXE := $(TEST_BUILD)/test_drivers.exe
# Compile driver thật cùng test harness, dùng header giả để không truy cập địa chỉ ngoại vi.
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

# -Werror yêu cầu test trên PC không có cảnh báo biên dịch.
$(TEST_EXE): $(TEST_SOURCES) $(TEST_HEADERS) | $(TEST_BUILD)
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror \
	    -I$(TEST_DIR)/fakes -I$(COMMON_DIR)/../BT_1_FreeRTOS/inc \
	    -I$(COMMON_DIR)/../BT_2_FreeRTOS/inc $(TEST_SOURCES) -o "$@"

test: $(TEST_EXE)
	"$(TEST_EXE)"
