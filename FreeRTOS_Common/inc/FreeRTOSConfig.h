#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

#define configUSE_PREEMPTION                     1
#define configUSE_TIME_SLICING                   1
#define configCPU_CLOCK_HZ                      64000000UL
#define configTICK_RATE_HZ                      1000UL
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                128
#define configTOTAL_HEAP_SIZE                   (10U * 1024U)
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                   0
#define configIDLE_SHOULD_YIELD                  1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK             1
#define configCHECK_FOR_STACK_OVERFLOW           2
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0
#define configUSE_MUTEXES                       0
#define configUSE_RECURSIVE_MUTEXES              0
#define configUSE_COUNTING_SEMAPHORES            0
#define configUSE_QUEUE_SETS                    0
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_CO_ROUTINES                   0
#define configUSE_NEWLIB_REENTRANT               0

/* Bài 2 dùng software timer để chờ nút được thả ổn định. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               3
#define configTIMER_QUEUE_LENGTH                8
#define configTIMER_TASK_STACK_DEPTH             192

/* STM32F103: 4 bit ưu tiên, số nhỏ là ưu tiên cao. */
#define configPRIO_BITS                         4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY         (15U << (8U - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (5U << (8U - configPRIO_BITS))

#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelete                     0
#define INCLUDE_xTaskGetSchedulerState          1

/* Gắn handlers của port ARM_CM3 vào vector table STM32. */
#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler
#define xPortSysTickHandler                     SysTick_Handler

void RTOS_AssertFailed(const char *file, uint32_t line);
#define configASSERT(condition) \
    do { if (!(condition)) RTOS_AssertFailed(__FILE__, __LINE__); } while (0)

#endif
