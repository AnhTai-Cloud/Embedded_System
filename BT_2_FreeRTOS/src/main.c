/*
 * Điểm vào ứng dụng Bài 02.
 * main chỉ chuẩn bị phần cứng và FreeRTOS; công việc truyền dữ liệu nằm ở task.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "app.h"
#include "board.h"

int main(void)
{
    /* Chuẩn bị clock 64 MHz và nhóm ưu tiên ngắt trước khi tạo đối tượng RTOS. */
    Board_Init();
    /* Tạo Queue, cấu hình UART/EXTI và tạo UART_Task; task chưa chạy ở đây. */
    App_Init();
    /* Khởi động SysTick và bộ lập lịch, từ đây FreeRTOS điều phối các task. */
    vTaskStartScheduler();
    /* Bình thường scheduler không trả về; nếu có thì dừng để kiểm tra lỗi. */
    Board_Panic(BOARD_FAULT_SCHEDULER);
}
