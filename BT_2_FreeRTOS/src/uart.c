#include "board.h"
#include "gpio.h"
#include "uart.h"

void UART_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    GPIO_ConfigPin(GPIOA, 9U, 0xAUL); /* USART1_TX, AF push-pull. */
    USART1->CR1 = 0U;
    USART1->CR2 = 0U;
    USART1->CR3 = 0U;
    USART1->BRR = (BOARD_CPU_HZ + UART_BAUD / 2UL) / UART_BAUD;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE;
}

static void UART_SendChar(char value)
{
    while ((USART1->SR & USART_SR_TXE) == 0U)
    {
    }
    USART1->DR = (uint8_t)value;
}

void UART_SendString(const char *text)
{
    while (*text != '\0')
    {
        UART_SendChar(*text++);
    }
}

void UART_SendCount(uint32_t count)
{
    char digits[10]; /* uint32_t lớn nhất 4294967295: đúng 10 chữ số. */
    uint32_t length = 0U;
    UART_SendString("Count: ");
    do
    {
        digits[length++] = (char)('0' + count % 10UL);
        count /= 10UL;
    } while (count != 0U);
    while (length != 0U)
    {
        UART_SendChar(digits[--length]);
    }
    UART_SendString("\r\n");
}
