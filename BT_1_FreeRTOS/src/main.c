#include "FreeRTOS.h"
#include "task.h"
#include "app.h"
#include "board.h"

int main(void)
{
    Board_Init();
    App_Init();
    vTaskStartScheduler();
    Board_Panic(BOARD_FAULT_SCHEDULER);
}
