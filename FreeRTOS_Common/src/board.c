/* Clock 64 MHz và xử lý lỗi dùng chung cho hai ứng dụng FreeRTOS. */
#include "board.h"

/* Biến .bss bắt đầu bằng 0; Board_Panic ghi giá trị enum BoardFault khi có lỗi. */
volatile uint32_t board_fault_code;

/* Chờ các bit mask của thanh ghi bằng value, có giới hạn để không treo mãi.
 * timeout là số lượt kiểm tra, không phải một đơn vị thời gian mili giây.
 */
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

/* Chuẩn bị clock trước khi kernel cấu hình SysTick hoặc driver tính baud/PWM. */
void Board_Init(void)
{
    /* Chuyển về HSI trước khi thay cấu hình PLL. */
    RCC->CR |= RCC_CR_HSION;
    WaitForBit(&RCC->CR, RCC_CR_HSIRDY, RCC_CR_HSIRDY);
    /* Chọn HSI làm SYSCLK và chờ chuyển xong rồi mới tắt PLL cũ. */
    RCC->CFGR &= ~RCC_CFGR_SW;
    WaitForBit(&RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_HSI);
    RCC->CR &= ~RCC_CR_PLLON;
    WaitForBit(&RCC->CR, RCC_CR_PLLRDY, 0U);

    /* 64 MHz cần 2 wait states; không nhầm LATENCY bit mask với giá trị 2. */
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_PRFTBE | 2UL;
    /* PLL dùng HSI 8 MHz / 2 x 16 = 64 MHz. AHB và APB2 không chia,
     * APB1 chia 2 để không vượt giới hạn 36 MHz của bus này.
     */
    RCC->CFGR = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PLLMULL16;
    RCC->CR |= RCC_CR_PLLON;
    WaitForBit(&RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY);
    /* Chọn PLL làm SYSCLK; chỉ tiếp tục khi phần cứng đã xác nhận lựa chọn. */
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    WaitForBit(&RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_PLL);
    /* Tính lại biến SystemCoreClock của CMSIS theo thanh ghi vừa cấu hình. */
    SystemCoreClockUpdate();

    if (SystemCoreClock != BOARD_CPU_HZ)
    {
        Board_Panic(BOARD_FAULT_CLOCK);
    }
    /* Tất cả bit ưu tiên dùng làm preemption priority, đúng yêu cầu FreeRTOS. */
    NVIC_SetPriorityGrouping(0U);
    /* Bài 2 cần AFIO để ánh xạ EXTI và GPIOA cho PA0/PA9. */
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN;
}

/* Dừng có kiểm soát để người debug còn đọc được nguyên nhân lỗi. */
void Board_Panic(BoardFault fault)
{
    /* Khóa ngắt trước khi lưu lỗi để ISR không tiếp tục đổi trạng thái ứng dụng. */
    __disable_irq();
    board_fault_code = (uint32_t)fault;
    for (;;)
    {
        __NOP(); /* Đặt breakpoint tại đây để đọc lỗi. */
    }
}
