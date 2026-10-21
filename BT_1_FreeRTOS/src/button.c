#include "button.h"
#include "gpio.h"

#define BUTTON_COUNT 3U
#define DEBOUNCE_SAMPLES 4U

typedef struct
{
    uint8_t candidate;
    uint8_t stable;
    uint8_t samples;
} ButtonDebounce;

static ButtonDebounce buttons[BUTTON_COUNT];

void Button_Init(void)
{
    uint32_t pin;
    for (pin = 0U; pin < BUTTON_COUNT; ++pin)
    {
        GPIO_InputPullUp(GPIOA, pin);
        buttons[pin] = (ButtonDebounce){0U, 0U, 0U};
    }
}

uint8_t Button_ScanPressed(void)
{
    uint8_t events = 0U;
    uint32_t pin;
    for (pin = 0U; pin < BUTTON_COUNT; ++pin)
    {
        ButtonDebounce *button = &buttons[pin];
        uint8_t raw = GPIO_IsLow(GPIOA, pin);
        if (raw != button->candidate)
        {
            button->candidate = raw;
            button->samples = 1U;
        }
        else if (button->samples < DEBOUNCE_SAMPLES)
        {
            ++button->samples;
        }
        if ((button->samples >= DEBOUNCE_SAMPLES) && (button->stable != raw))
        {
            button->stable = raw;
            if (raw != 0U)
            {
                events |= (uint8_t)(1U << pin);
            }
        }
    }
    return events;
}
