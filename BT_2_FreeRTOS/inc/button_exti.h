/*
 * Giao diện nút PA0 tạo ngắt cạnh xuống.
 * Module này đếm lần nhấn trong ISR và gửi số đếm tới Queue của ứng dụng.
 */
#ifndef COUNTER_BUTTON_EXTI_H
#define COUNTER_BUTTON_EXTI_H

#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>

/* Các biến quan sát khi debug. volatile yêu cầu đọc/ghi trực tiếp bộ nhớ,
 * không thay thế cơ chế đồng bộ hay Queue của FreeRTOS.
 */
/* Tổng số lần nhấn đã được ISR chấp nhận sau cơ chế chống dội. */
extern volatile uint32_t button_press_count;
/* Số lần không gửi được số đếm vì Queue nhận đã đầy. */
extern volatile uint32_t button_queue_dropped;
/* Số lần không gửi được lệnh khởi động timer từ ISR. */
extern volatile uint32_t button_timer_errors;

/* Nhận handle Queue đã tạo trong App_Init(), cấu hình PA0/EXTI và timer.
 * Hàm này chưa cho phép ngắt chạy vì scheduler chưa được khởi động.
 */
void Button_EXTI_Init(QueueHandle_t queue);
/* Gọi từ UART_Task sau khi scheduler đã chạy, bắt đầu nhận lần nhấn. */
void Button_EXTI_Enable(void);
/* Tên hàm phải khớp vector EXTI0 trong startup STM32; phần cứng tự gọi. */
void EXTI0_IRQHandler(void);

#endif
