#include "led_control.h"

void LED_ControlInit(LED_ControlState *state)
{
    state->selected_led = 0U;
    state->brightness[0] = 50U;
    state->brightness[1] = 50U;
    state->brightness[2] = 50U;
}

uint8_t LED_ControlApply(LED_ControlState *state, uint8_t pressed_mask)
{
    uint8_t changed = 0U;
    uint8_t *brightness;
    if ((pressed_mask & BUTTON_SELECT_MASK) != 0U)
    {
        state->selected_led = (uint8_t)((state->selected_led + 1U) % CONTROL_LED_COUNT);
        changed = 1U;
    }
    brightness = &state->brightness[state->selected_led];

    /* Nếu tăng và giảm cùng nhấn, độ sáng không đổi. */
    if ((pressed_mask & (BUTTON_UP_MASK | BUTTON_DOWN_MASK)) == BUTTON_UP_MASK)
    {
        if (*brightness < 100U)
        {
            *brightness = (*brightness > (100U - CONTROL_BRIGHTNESS_STEP))
                ? 100U : (uint8_t)(*brightness + CONTROL_BRIGHTNESS_STEP);
            changed = 1U;
        }
    }
    else if ((pressed_mask & (BUTTON_UP_MASK | BUTTON_DOWN_MASK)) == BUTTON_DOWN_MASK)
    {
        if (*brightness > 0U)
        {
            *brightness = (*brightness < CONTROL_BRIGHTNESS_STEP)
                ? 0U : (uint8_t)(*brightness - CONTROL_BRIGHTNESS_STEP);
            changed = 1U;
        }
    }
    return changed;
}
