/* Giao diện phần cứng dùng chung: clock hệ thống và cơ chế dừng khi có lỗi. */
#ifndef FREERTOS_BOARD_H
#define FREERTOS_BOARD_H

#include "stm32f1xx.h"
#include <stdint.h>

/* HCLK và PCLK2 đều 64 MHz; FreeRTOSConfig.h phải dùng cùng tần số này. */
#define BOARD_CPU_HZ 64000000UL

/* Mã lỗi được lưu vào board_fault_code để đọc bằng debugger. */
typedef enum
{
    BOARD_FAULT_CLOCK = 1, /* Clock không sẵn sàng hoặc sai tần số yêu cầu. */
    BOARD_FAULT_ALLOC,     /* FreeRTOS cấp phát heap thất bại. */
    BOARD_FAULT_STACK,     /* Phát hiện stack của một task bị tràn. */
    BOARD_FAULT_ASSERT,    /* Một điều kiện configASSERT không được thỏa mãn. */
    BOARD_FAULT_SCHEDULER, /* Scheduler trả về dù ứng dụng không cho phép. */
    BOARD_FAULT_CREATE     /* Không tạo được task, Queue hoặc software timer. */
} BoardFault;

/* Giá trị 0 khi chưa ghi nhận lỗi; volatile giúp quan sát đúng khi debug. */
extern volatile uint32_t board_fault_code;

/* Khởi tạo HSI/2 x 16 = 64 MHz, APB1 = 32 MHz, APB2 = 64 MHz.
 * Gọi trước App_Init và trước khi khởi động scheduler.
 */
void Board_Init(void);
/* Khóa ngắt, lưu mã lỗi và dừng vô hạn. noreturn báo hàm không quay lại. */
void Board_Panic(BoardFault fault) __attribute__((noreturn));

#endif
