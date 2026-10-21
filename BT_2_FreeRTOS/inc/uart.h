#ifndef COUNTER_UART_H
#define COUNTER_UART_H

#include <stdint.h>

#define UART_BAUD 115200UL

void UART_Init(void);
void UART_SendString(const char *text);
/* Chuỗi định dạng ASCII: Count: <số>\\r\\n */
void UART_SendCount(uint32_t count);

#endif
