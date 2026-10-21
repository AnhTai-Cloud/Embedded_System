#include "board.h"

volatile uint32_t board_fault_code;

static void WaitForBit(volatile uint32_t *reg, uint32_t mask, uint32_t value)
{
    uint32_t timeout = 2000000UL;
    while ((*reg & mask) != value)
    {
        if (--timeout == 0U)
        {
            Board_Panic(BOARD_FAULT_CLOCK);
        }
    }
}

void Board_Init(void)
{
    /* Chuyển về HSI trước khi thay cấu hình PLL. */
    RCC->CR |= RCC_CR_HSION;
    WaitForBit(&RCC->CR, RCC_CR_HSIRDY, RCC_CR_HSIRDY);
    RCC->CFGR &= ~RCC_CFGR_SW;
    WaitForBit(&RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_HSI);
    RCC->CR &= ~RCC_CR_PLLON;
    WaitForBit(&RCC->CR, RCC_CR_PLLRDY, 0U);

    /* 64 MHz cần 2 wait states; không nhầm LATENCY bit mask với giá trị 2. */
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_PRFTBE | 2UL;
    RCC->CFGR = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PLLMULL16;
    RCC->CR |= RCC_CR_PLLON;
    WaitForBit(&RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY);
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    WaitForBit(&RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_PLL);
    SystemCoreClockUpdate();

    if (SystemCoreClock != BOARD_CPU_HZ)
    {
        Board_Panic(BOARD_FAULT_CLOCK);
    }
    /* Tất cả bit ưu tiên dùng làm preemption priority, đúng yêu cầu FreeRTOS. */
    NVIC_SetPriorityGrouping(0U);
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN;
}

void Board_Panic(BoardFault fault)
{
    __disable_irq();
    board_fault_code = (uint32_t)fault;
    for (;;)
    {
        __NOP(); /* Đặt breakpoint tại đây để đọc lỗi. */
    }
}
