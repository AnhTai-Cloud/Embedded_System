/*
 * Tầng ứng dụng: tạo Queue và một task UART.
 * EXTI0_IRQHandler là bên gửi; UART_Task là bên nhận dữ liệu.
 * Module button_exti giữ bản sao handle của cùng Queue, không tạo Queue thứ hai.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "board.h"
#include "app.h"
#include "button_exti.h"
#include "uart.h"

/* Handle dùng để truy cập Queue; static giới hạn tên biến trong file này. */
static QueueHandle_t count_queue;
/* Lưu số đếm mới nhất đã được task nhận, thuận tiện quan sát bằng debugger. */
volatile uint32_t uart_last_count;

/* Task có vòng lặp vô hạn và không được return về hàm đã tạo nó. */
static void UART_Task(void *argument)
{
    /* Queue chép giá trị nhận vào biến cục bộ này. */
    uint32_t count;
    /* Task hiện không cần tham số; ép void để tránh cảnh báo biến không dùng. */
    (void)argument;
    /* Gửi thông báo khởi động và số đếm ban đầu để kiểm tra kết nối terminal. */
    UART_SendString("\r\nSTM32F103 FreeRTOS EXTI -> Queue -> UART1\r\n");
    UART_SendCount(0U);
    /* Bật ngắt ở trong task để bảo đảm Queue, tick và timer đã sẵn sàng. */
    Button_EXTI_Enable();

    for (;;)
    {
        /*
         * Queue trống: task chuyển sang Blocked, nhường CPU cho task khác.
         * portMAX_DELAY cho phép chờ vô hạn với cấu hình FreeRTOS hiện tại.
         * Có dữ liệu: lấy phần tử đầu Queue theo FIFO, chép vào count.
         */
        if (xQueueReceive(count_queue, &count, portMAX_DELAY) == pdPASS)
        {
            /* Chỉ truyền sau khi nhận thành công; thao tác UART nằm ngoài ISR. */
            uart_last_count = count;
            UART_SendCount(count);
        }
    }
}

/* Chuẩn bị toàn bộ ứng dụng trước khi main khởi động scheduler. */
void App_Init(void)
{
    UART_Init();
    /* Queue giữ tối đa 16 số uint32_t. sizeof là kích thước mỗi phần tử,
     * không phải tổng kích thước; Queue sao chép dữ liệu thay vì giữ con trỏ count.
     */
    count_queue = xQueueCreate(16U, sizeof(uint32_t));
    if (count_queue == NULL)
    {
        /* Không đủ heap để tạo Queue: dừng để tránh dùng handle NULL. */
        Board_Panic(BOARD_FAULT_CREATE);
    }
    /* Truyền cùng handle sang module nút; lúc này EXTI vẫn đang bị khóa. */
    Button_EXTI_Init(count_queue);
    /* 256 là số word stack 32 bit (1024 byte), 2 là ưu tiên task.
     * Tham số task và nơi nhận task handle đều không dùng nên để NULL.
     */
    if (xTaskCreate(UART_Task, "UART1", 256U, NULL, 2U, NULL) != pdPASS)
    {
        Board_Panic(BOARD_FAULT_CREATE);
    }
}
