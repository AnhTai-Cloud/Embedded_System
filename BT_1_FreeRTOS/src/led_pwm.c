#include "board.h"
#include "gpio.h"
#include "led_pwm.h"

void LED_PWM_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    RCC->APB2RSTR |= RCC_APB2RSTR_TIM1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM1RST;
    /* PA8/PA9/PA10: TIM1_CH1/CH2/CH3, alternate function push-pull 2 MHz. */
    GPIO_ConfigPin(GPIOA, 8U, 0xAUL);
    GPIO_ConfigPin(GPIOA, 9U, 0xAUL);
    GPIO_ConfigPin(GPIOA, 10U, 0xAUL);

    /* TIM1 = PCLK2 = 64 MHz: 64 MHz / 64 / 1000 = 1000 Hz. */
    TIM1->PSC = (BOARD_CPU_HZ / (LED_PWM_HZ * LED_PWM_PERIOD_COUNTS)) - 1UL;
    TIM1->ARR = LED_PWM_PERIOD_COUNTS - 1UL;
    TIM1->CCR1 = 0U;
    TIM1->CCR2 = 0U;
    TIM1->CCR3 = 0U;
    TIM1->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE |
                   TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE;
    TIM1->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE;

    /*
     * PWM mode 1 + polarity LOW: khoảng CNT < CCR phát LOW để LED sáng.
     * CCR=0: luôn HIGH/tắt; CCR=1000: luôn LOW/sáng tối đa.
     */
    TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P |
                 TIM_CCER_CC2E | TIM_CCER_CC2P |
                 TIM_CCER_CC3E | TIM_CCER_CC3P;
    TIM1->CR1 = TIM_CR1_ARPE;
    TIM1->EGR = TIM_EGR_UG;
    TIM1->SR = 0U;
    TIM1->BDTR = TIM_BDTR_MOE; /* Advanced timer TIM1 cần bật MOE. */
    TIM1->CR1 |= TIM_CR1_CEN;
}

void LED_PWM_SetBrightness(uint8_t led_index, uint8_t percent)
{
    uint32_t compare;
    if (led_index >= LED_COUNT)
    {
        return;
    }
    if (percent > 100U)
    {
        percent = 100U;
    }
    compare = (uint32_t)percent * LED_PWM_PERIOD_COUNTS / 100UL;
    switch (led_index)
    {
        case 0U: TIM1->CCR1 = compare; break;
        case 1U: TIM1->CCR2 = compare; break;
        case 2U: TIM1->CCR3 = compare; break;
        default: break;
    }
}
