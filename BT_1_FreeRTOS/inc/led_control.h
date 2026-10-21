#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <stdint.h>

#define CONTROL_LED_COUNT 3U
#define CONTROL_BRIGHTNESS_STEP 10U
#define BUTTON_SELECT_MASK  (1U << 0)
#define BUTTON_UP_MASK      (1U << 1)
#define BUTTON_DOWN_MASK    (1U << 2)

typedef struct
{
    uint8_t selected_led; /* 0..2 tương ứng LED1..LED3. */
    uint8_t brightness[CONTROL_LED_COUNT];
} LED_ControlState;

void LED_ControlInit(LED_ControlState *state);
/* Trả về 1 nếu selection/brightness đã thay đổi. */
uint8_t LED_ControlApply(LED_ControlState *state, uint8_t pressed_mask);

#endif
