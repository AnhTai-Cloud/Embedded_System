#ifndef TEST_GPIO_H
#define TEST_GPIO_H
#include "board.h"
void GPIO_ConfigPin(GPIO_TypeDef *port, uint32_t pin, uint32_t configuration);
void GPIO_InputPullUp(GPIO_TypeDef *port, uint32_t pin);
uint8_t GPIO_IsLow(GPIO_TypeDef *port, uint32_t pin);
#endif
