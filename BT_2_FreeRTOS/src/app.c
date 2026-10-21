#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "board.h"
#include "app.h"
#include "button_exti.h"
#include "uart.h"

static QueueHandle_t count_queue;
volatile uint32_t uart_last_count;

static void UART_Task(void *argument)
{
    uint32_t count;
    (void)argument;
    UART_SendString("\r\nSTM32F103 FreeRTOS EXTI -> Queue -> UART1\r\n");
    UART_SendCount(0U);
    Button_EXTI_Enable();

    for (;;)
    {
        /* Blocked cho tới khi ISR gửi dữ liệu; không polling queue. */
        if (xQueueReceive(count_queue, &count, portMAX_DELAY) == pdPASS)
        {
            uart_last_count = count;
            UART_SendCount(count);
        }
    }
}

void App_Init(void)
{
    UART_Init();
    count_queue = xQueueCreate(16U, sizeof(uint32_t));
    if (count_queue == NULL)
    {
        Board_Panic(BOARD_FAULT_CREATE);
    }
    Button_EXTI_Init(count_queue);
    if (xTaskCreate(UART_Task, "UART1", 256U, NULL, 2U, NULL) != pdPASS)
    {
        Board_Panic(BOARD_FAULT_CREATE);
    }
}
