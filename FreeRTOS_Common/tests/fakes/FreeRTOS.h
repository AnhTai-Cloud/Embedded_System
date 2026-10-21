#ifndef TEST_FREERTOS_H
#define TEST_FREERTOS_H
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
typedef int BaseType_t;
typedef uint32_t TickType_t;
#define pdFALSE 0
#define pdTRUE 1
#define pdPASS 1
#define pdFAIL 0
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5U
#define configASSERT(condition) assert(condition)
void Test_RecordYield(BaseType_t woken);
#define portYIELD_FROM_ISR(woken) Test_RecordYield(woken)
#endif
