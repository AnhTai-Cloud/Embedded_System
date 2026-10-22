/* Giao diện khởi tạo ứng dụng Bài 02: EXTI -> Queue -> task UART. */
#ifndef COUNTER_APP_H
#define COUNTER_APP_H

/* Tạo UART, Queue, bộ chống dội và task; gọi trước vTaskStartScheduler(). */
void App_Init(void);

#endif
