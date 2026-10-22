/* Thao tác GPIO STM32F1 trực tiếp trên CRL/CRH, BSRR và IDR. */
#include "gpio.h"

/* Mỗi chân STM32F1 có 4 bit cấu hình: 2 bit MODE và 2 bit CNF. */
void GPIO_ConfigPin(GPIO_TypeDef *port, uint32_t pin, uint32_t configuration)
{
    /* Chân 0..7 thuộc CRL, chân 8..15 thuộc CRH. */
    volatile uint32_t *reg = (pin < 8U) ? &port->CRL : &port->CRH;
    /* Đưa số chân về vị trí 0..7 trong thanh ghi đã chọn, rồi đổi sang bit. */
    uint32_t shift = (pin & 7U) * 4U;
    /* Xóa đúng nibble của chân cần cấu hình rồi chèn giá trị mới, giữ chân khác. */
    *reg = (*reg & ~(0xFUL << shift)) | ((configuration & 0xFUL) << shift);
}

void GPIO_InputPullUp(GPIO_TypeDef *port, uint32_t pin)
{
    /* Trong chế độ input pull-up/down, bit ODR chọn hướng kéo.
     * BSRR đặt ODR=1 trước khi chuyển mode để chọn điện trở kéo lên nội.
     */
    port->BSRR = 1UL << pin; /* ODR=1 chọn pull-up, không phải output. */
    /* 0x8: MODE=00 là input, CNF=10 là input có pull-up/pull-down. */
    GPIO_ConfigPin(port, pin, 0x8UL);
}

uint8_t GPIO_IsLow(GPIO_TypeDef *port, uint32_t pin)
{
    /* Đọc mức điện áp từ IDR, không đọc ODR vì ODR chỉ giữ giá trị chốt đầu ra. */
    return (uint8_t)((port->IDR & (1UL << pin)) == 0U);
}
