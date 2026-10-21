#include "led_blink.h"

uint8_t LED_Blink_Init(LED_BlinkState *state, float frequency_hz,
                       uint32_t tick_rate_hz, uint32_t now)
{
    uint32_t half_period = 0U;

    if (tick_rate_hz == 0U)
    {
        return 0U;
    }
    if (frequency_hz != 0.0f)
    {
        float ticks;
        /* So sánh này cũng loại NaN và infinity, tránh ép float lỗi sang số nguyên. */
        if (!(frequency_hz >= 0.01f && frequency_hz <= 100.0f))
        {
            return 0U;
        }
        ticks = (float)tick_rate_hz / (2.0f * frequency_hz);
        if (!(ticks >= 1.0f && ticks < 2147483520.0f))
        {
            return 0U;
        }
        half_period = (uint32_t)(ticks + 0.5f);
    }

    state->half_period_ticks = half_period;
    state->phase_start = now;
    state->is_on = 1U;
    return 1U;
}

uint32_t LED_Blink_Update(LED_BlinkState *state, uint32_t now)
{
    uint32_t elapsed;
    uint32_t phases;

    if (state->half_period_ticks == 0U)
    {
        return UINT32_MAX;
    }
    /* Phép trừ unsigned vẫn đúng khi tick counter uint32_t quay vòng. */
    elapsed = now - state->phase_start;
    phases = elapsed / state->half_period_ticks;
    if (phases != 0U)
    {
        state->phase_start += phases * state->half_period_ticks;
        state->is_on ^= (uint8_t)(phases & 1U);
    }
    return state->half_period_ticks - (now - state->phase_start);
}
