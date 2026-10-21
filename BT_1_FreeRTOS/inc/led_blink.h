#ifndef LED_BLINK_H
#define LED_BLINK_H

#include <stdint.h>

typedef struct
{
    uint32_t half_period_ticks;
    uint32_t phase_start;
    uint8_t is_on;
} LED_BlinkState;

/* Hàm chung cho mọi LED: 0 Hz = sáng liên tục; hỗ trợ nháy 0.01..100 Hz.
 * Tick và now lấy từ FreeRTOS. Trả về 0 nếu tham số không hợp lệ.
 */
uint8_t LED_Blink_Init(LED_BlinkState *state, float frequency_hz,
                       uint32_t tick_rate_hz, uint32_t now);

/* Cập nhật pha và trả về số tick tới lần chuyển pha tiếp theo.
 * UINT32_MAX = không nháy. Mỗi LED giữ một state độc lập.
 */
uint32_t LED_Blink_Update(LED_BlinkState *state, uint32_t now);

#endif
