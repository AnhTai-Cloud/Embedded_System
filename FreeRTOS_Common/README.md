# Phần dùng chung cho hai bài FreeRTOS

- inc/FreeRTOSConfig.h: tick 1 kHz, preemption, heap 10 KiB, priorities và IRQ handlers.
- inc/board.h + src/board.c: clock HSI/2 x 16 = 64 MHz; APB1 32 MHz, APB2 64 MHz.
- inc/gpio.h + src/gpio.c: primitive GPIO và input pull-up.
- src/rtos_hooks.c: malloc/stack/assert failure hooks.
- STM32F103C8.ld: FLASH 64 KiB, RAM 20 KiB; dành 2 KiB cho main/ISR stack.
- build.mk: build mỗi bài vào build/ riêng, target flash và test.
- tests/: native PC tests với driver thật và thanh ghi/API giả.

Không chỉnh file kernel của STM32CubeF1. Hai firmware sử dụng port GCC ARM_CM3 và heap_4.
Mapping SVC/PendSV/SysTick nằm trong FreeRTOSConfig.h, bảo đảm vector table gọi port thật.
Thư viện ngoại vi dùng CMSIS trực tiếp giống BT_3; FreeRTOS quản lý task/queue/timer.

Chạy make -C BT_1_FreeRTOS test để kiểm tra:
selection và độ sáng độc lập, giới hạn 0/100%, chống dội/giữ nút,
PWM TIM1 1 kHz và các endpoint CCR, ba nhịp nháy 0.1/1/10 Hz độc lập,
tick quay vòng, EXTI gửi queue/yield và queue-full counter.

Các test giả lập API phục vụ kiểm tra logic, không mô phỏng kernel hoặc thời gian phần cứng.
Tần số PWM/baud ghi trong README là tần số danh định theo clock HSI.
