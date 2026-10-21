#include "FreeRTOS.h"
#include "task.h"
#include "board.h"

const char * volatile rtos_assert_file;
volatile uint32_t rtos_assert_line;

void RTOS_AssertFailed(const char *file, uint32_t line)
{
    rtos_assert_file = file;
    rtos_assert_line = line;
    Board_Panic(BOARD_FAULT_ASSERT);
}

void vApplicationMallocFailedHook(void)
{
    Board_Panic(BOARD_FAULT_ALLOC);
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task;
    (void)name;
    Board_Panic(BOARD_FAULT_STACK);
}
