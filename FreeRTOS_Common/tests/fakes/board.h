#ifndef TEST_BOARD_H
#define TEST_BOARD_H
#include <stdint.h>
typedef struct { uint32_t CRL, CRH, IDR, ODR, BSRR, BRR; } GPIO_TypeDef;
typedef struct { uint32_t APB2ENR, APB2RSTR; } RCC_TypeDef;
typedef struct
{
    uint32_t PSC, ARR, CCR1, CCR2, CCR3, CCMR1, CCMR2, CCER, CR1, EGR, SR, BDTR;
} TIM_TypeDef;
typedef struct { uint32_t EXTICR[4]; } AFIO_TypeDef;
typedef struct { uint32_t IMR, RTSR, FTSR, PR; } EXTI_TypeDef;
extern GPIO_TypeDef test_gpioa;
extern RCC_TypeDef test_rcc;
extern TIM_TypeDef test_tim1;
extern AFIO_TypeDef test_afio;
extern EXTI_TypeDef test_exti;
#define GPIOA (&test_gpioa)
#define RCC (&test_rcc)
#define TIM1 (&test_tim1)
#define AFIO (&test_afio)
#define EXTI (&test_exti)
#define BOARD_CPU_HZ 64000000UL
#define BOARD_FAULT_CREATE 6
#define RCC_APB2ENR_TIM1EN (1UL << 11)
#define RCC_APB2RSTR_TIM1RST (1UL << 11)
#define TIM_CCMR1_OC1M_1 (1UL << 5)
#define TIM_CCMR1_OC1M_2 (1UL << 6)
#define TIM_CCMR1_OC1PE (1UL << 3)
#define TIM_CCMR1_OC2M_1 (1UL << 13)
#define TIM_CCMR1_OC2M_2 (1UL << 14)
#define TIM_CCMR1_OC2PE (1UL << 11)
#define TIM_CCMR2_OC3M_1 (1UL << 5)
#define TIM_CCMR2_OC3M_2 (1UL << 6)
#define TIM_CCMR2_OC3PE (1UL << 3)
#define TIM_CCER_CC1E (1UL << 0)
#define TIM_CCER_CC1P (1UL << 1)
#define TIM_CCER_CC2E (1UL << 4)
#define TIM_CCER_CC2P (1UL << 5)
#define TIM_CCER_CC3E (1UL << 8)
#define TIM_CCER_CC3P (1UL << 9)
#define TIM_CR1_ARPE (1UL << 7)
#define TIM_CR1_CEN (1UL << 0)
#define TIM_EGR_UG (1UL << 0)
#define TIM_BDTR_MOE (1UL << 15)
#define AFIO_EXTICR1_EXTI0 0xFUL
#define EXTI0_IRQn 6
void Board_Panic(int fault);
void NVIC_SetPriority(int irq, uint32_t priority);
void NVIC_ClearPendingIRQ(int irq);
void NVIC_EnableIRQ(int irq);
#endif
