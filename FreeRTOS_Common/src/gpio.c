#include "gpio.h"

void GPIO_ConfigPin(GPIO_TypeDef *port, uint32_t pin, uint32_t configuration)
{
    volatile uint32_t *reg = (pin < 8U) ? &port->CRL : &port->CRH;
    uint32_t shift = (pin & 7U) * 4U;
    *reg = (*reg & ~(0xFUL << shift)) | ((configuration & 0xFUL) << shift);
}

void GPIO_InputPullUp(GPIO_TypeDef *port, uint32_t pin)
{
    port->BSRR = 1UL << pin; /* ODR=1 chọn pull-up, không phải output. */
    GPIO_ConfigPin(port, pin, 0x8UL);
}

uint8_t GPIO_IsLow(GPIO_TypeDef *port, uint32_t pin)
{
    return (uint8_t)((port->IDR & (1UL << pin)) == 0U);
}
