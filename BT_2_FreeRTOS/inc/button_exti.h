#ifndef COUNTER_BUTTON_EXTI_H
#define COUNTER_BUTTON_EXTI_H

#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>

extern volatile uint32_t button_press_count;
extern volatile uint32_t button_queue_dropped;
extern volatile uint32_t button_timer_errors;

/* Queue phải được tạo trước; EXTI chỉ bật sau khi scheduler chạy. */
void Button_EXTI_Init(QueueHandle_t queue);
void Button_EXTI_Enable(void);
void EXTI0_IRQHandler(void);

#endif
