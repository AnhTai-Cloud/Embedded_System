# Makefile của Bài 02: giữ đường dẫn theo vị trí file để chạy được từ mọi thư mục.
PROJECT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
# Toàn bộ quy tắc biên dịch, test và nạp ST-Link nằm trong file dùng chung này.
include $(PROJECT_DIR)/../FreeRTOS_Common/build.mk
