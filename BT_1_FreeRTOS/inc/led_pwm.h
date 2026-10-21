#ifndef LED_PWM_H
#define LED_PWM_H

#include <stdint.h>

#define LED_COUNT 3U
#define LED_PWM_HZ 1000UL
#define LED_PWM_PERIOD_COUNTS 1000UL

void LED_PWM_Init(void);
void LED_PWM_SetBrightness(uint8_t led_index, uint8_t percent);

#endif
