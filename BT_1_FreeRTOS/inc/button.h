#ifndef LED_BUTTON_H
#define LED_BUTTON_H

#include <stdint.h>

#define BUTTON_SCAN_MS 10U

void Button_Init(void);
/* Gọi mỗi 10 ms; trả về mask sự kiện nhấn mới, chống dội 30 ms. */
uint8_t Button_ScanPressed(void);

#endif
