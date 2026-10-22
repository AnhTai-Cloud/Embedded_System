/*
 * Cấu hình kernel cho STM32F103 Cortex-M3 dùng ở cả hai bài.
 * Đây là file cấu hình của ứng dụng; không sửa mã nguồn kernel bên thứ ba.
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

/* Cho phép task ưu tiên cao giành CPU, và chia lượt giữa task cùng ưu tiên. */
#define configUSE_PREEMPTION                     1
#define configUSE_TIME_SLICING                   1
/* Clock phải khớp Board_Init. Tick 1000 Hz tương ứng 1 tick = 1 ms. */
#define configCPU_CLOCK_HZ                      64000000UL
#define configTICK_RATE_HZ                      1000UL
/* Có 5 mức ưu tiên task 0..4; số lớn hơn có ưu tiên cao hơn. */
#define configMAX_PRIORITIES                    5
/* Stack tính theo word 32 bit, heap tính theo byte.
 * Heap 10 KiB do heap_4 quản lý, chứa stack task và các đối tượng RTOS.
 */
#define configMINIMAL_STACK_SIZE                128
#define configTOTAL_HEAP_SIZE                   (10U * 1024U)
/* Giới hạn độ dài tên task để lưu/debug; dùng TickType_t 32 bit. */
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                   0
/* Idle nhường CPU cho task cùng ưu tiên khi cần; không dùng callback Idle/Tick. */
#define configIDLE_SHOULD_YIELD                  1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
/* Bật callback báo lỗi cấp phát và kiểm tra stack kiểu 2 khi chuyển task. */
#define configUSE_MALLOC_FAILED_HOOK             1
#define configCHECK_FOR_STACK_OVERFLOW           2
/* xTaskCreate/xQueueCreate/xTimerCreate dùng cấp phát động từ heap_4. */
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0
/* Không cần mutex, semaphore đếm, queue set hoặc coroutine trong hai bài. */
#define configUSE_MUTEXES                       0
#define configUSE_RECURSIVE_MUTEXES              0
#define configUSE_COUNTING_SEMAPHORES            0
#define configUSE_QUEUE_SETS                    0
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_CO_ROUTINES                   0
/* Không cấp vùng newlib riêng cho từng task; driver UART không dùng printf. */
#define configUSE_NEWLIB_REENTRANT               0

/* Software timer của Bài 2 chạy callback trong Timer Service task.
 * Priority 3 cao hơn UART task priority 2. Queue dài 8 giữ các lệnh timer,
 * khác với Queue 16 số đếm được tạo trong App_Init.
 */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               3
#define configTIMER_QUEUE_LENGTH                8
#define configTIMER_TASK_STACK_DEPTH             192

/* Ưu tiên NGẮT NVIC: STM32F103 có 4 bit, số nhỏ lại có ưu tiên cao hơn.
 * ISR gọi API FromISR phải có số ưu tiên >= 5; EXTI0 được đặt bằng 5.
 * Quy tắc này khác với ưu tiên TASK ở phía trên, không so sánh hai thang với nhau.
 */
#define configPRIO_BITS                         4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
/* Dịch 4 bit để đặt giá trị vào các bit ưu tiên thực sự của thanh ghi NVIC. */
#define configKERNEL_INTERRUPT_PRIORITY         (15U << (8U - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (5U << (8U - configPRIO_BITS))

/* Bật các API được ứng dụng/kernel sử dụng.
 * INCLUDE_vTaskSuspend=1 cũng cho phép portMAX_DELAY chờ vô hạn ở Queue.
 */
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelete                     0
#define INCLUDE_xTaskGetSchedulerState          1

/* Tên hàm của port ARM_CM3 được đổi thành tên có trong vector table STM32:
 * SVC bắt đầu task đầu tiên, PendSV đổi ngữ cảnh, SysTick tạo nhịp kernel.
 */
#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler
#define xPortSysTickHandler                     SysTick_Handler

/* Khi điều kiện sai: lưu file/dòng tại rtos_hooks.c rồi dừng để debug. */
void RTOS_AssertFailed(const char *file, uint32_t line);
#define configASSERT(condition) \
    do { if (!(condition)) RTOS_AssertFailed(__FILE__, __LINE__); } while (0)

#endif
