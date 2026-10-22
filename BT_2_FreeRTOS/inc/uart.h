/* Giao diện UART1: chỉ truyền trên PA9 tới RX của USB-UART mức 3.3 V. */
#ifndef COUNTER_UART_H
#define COUNTER_UART_H

#include <stdint.h>

/* Tốc độ baud danh định; terminal dùng 115200 baud, 8N1, không flow control. */
#define UART_BAUD 115200UL

/* Cấu hình PA9, clock và USART1; gọi sau Board_Init(). */
void UART_Init(void);
/* Gửi chuỗi kết thúc bằng ký tự '\0'; không truyền chính ký tự kết thúc. */
void UART_SendString(const char *text);
/* Định dạng số đếm thành chuỗi "Count: <số>" rồi thêm CR và LF xuống dòng. */
void UART_SendCount(uint32_t count);

#endif
