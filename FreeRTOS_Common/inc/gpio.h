/* Các thao tác GPIO mức thanh ghi, dùng chung cho nút, PWM và UART. */
#ifndef FREERTOS_GPIO_H
#define FREERTOS_GPIO_H

#include "stm32f1xx.h"
#include <stdint.h>

/* Cấu hình một chân 0..15 bằng nibble MODE/CNF; không đổi chân khác.
 * Bên gọi phải cấp clock cho port trước khi sử dụng.
 */
void GPIO_ConfigPin(GPIO_TypeDef *port, uint32_t pin, uint32_t configuration);
/* Input pull-up: để hở đọc HIGH, nối nút xuống GND thì đọc LOW. */
void GPIO_InputPullUp(GPIO_TypeDef *port, uint32_t pin);
/* Trả về 1 nếu điện áp đang được đọc là LOW, ngược lại trả về 0. */
uint8_t GPIO_IsLow(GPIO_TypeDef *port, uint32_t pin);

#endif
