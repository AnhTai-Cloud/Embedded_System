/*
 * Host tests chạy code driver thật với thanh ghi/API giả.
 * Kiểm tra logic controller, debounce, PWM endpoint và ISR/queue/yield.
 * Không thay thế kiểm tra scheduler/PWM điện áp trên STM32.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "board.h"
#include "gpio.h"
#include "led_control.h"
#include "led_blink.h"
#include "led_pwm.h"
#include "button.h"
#include "button_exti.h"
#include "timers.h"

GPIO_TypeDef test_gpioa;
RCC_TypeDef test_rcc;
TIM_TypeDef test_tim1;
AFIO_TypeDef test_afio;
EXTI_TypeDef test_exti;
static TimerCallbackFunction_t release_callback;
static uint32_t last_sent, sends, yields, timer_resets, irq_priority;
static BaseType_t queue_result = pdPASS;

void Board_Panic(int fault) { (void)fault; abort(); }
void NVIC_SetPriority(int irq, uint32_t priority) { (void)irq; irq_priority = priority; }
void NVIC_ClearPendingIRQ(int irq) { (void)irq; }
void NVIC_EnableIRQ(int irq) { (void)irq; }

void GPIO_ConfigPin(GPIO_TypeDef *port, uint32_t pin, uint32_t config)
{
    uint32_t *reg = pin < 8U ? &port->CRL : &port->CRH;
    uint32_t shift = (pin & 7U) * 4U;
    *reg = (*reg & ~(0xFUL << shift)) | ((config & 0xFUL) << shift);
}
void GPIO_InputPullUp(GPIO_TypeDef *port, uint32_t pin)
{
    port->ODR |= 1UL << pin;
    GPIO_ConfigPin(port, pin, 8U);
}
uint8_t GPIO_IsLow(GPIO_TypeDef *port, uint32_t pin)
{
    return (uint8_t)((port->IDR & (1UL << pin)) == 0U);
}
BaseType_t xQueueSendFromISR(QueueHandle_t queue, const void *item, BaseType_t *woken)
{
    assert(queue != NULL);
    if (queue_result == pdPASS)
    {
        last_sent = *(const uint32_t *)item;
        ++sends;
        *woken = pdTRUE; /* Mô phỏng task nhận đang Blocked và có priority cao hơn. */
    }
    return queue_result;
}
void Test_RecordYield(BaseType_t woken)
{
    if (woken != pdFALSE) ++yields;
}
TimerHandle_t xTimerCreate(const char *name, TickType_t ticks, BaseType_t reload,
                          void *id, TimerCallbackFunction_t callback)
{
    (void)name; (void)id;
    assert(ticks == 10U && reload == pdFALSE);
    release_callback = callback;
    return &timer_resets;
}
BaseType_t xTimerReset(TimerHandle_t timer, TickType_t block_time)
{
    assert(timer != NULL && block_time == 0U);
    ++timer_resets;
    return pdPASS;
}
BaseType_t xTimerResetFromISR(TimerHandle_t timer, BaseType_t *woken)
{
    assert(timer != NULL && woken != NULL);
    ++timer_resets;
    return pdPASS;
}

static void Test_Control(void)
{
    LED_ControlState state;
    LED_ControlInit(&state);
    assert(state.selected_led == 0U && state.brightness[0] == 50U);
    assert(state.brightness[1] == 50U && state.brightness[2] == 50U);
    for (unsigned i = 0; i < 5U; ++i)
        assert(LED_ControlApply(&state, BUTTON_UP_MASK) != 0U);
    assert(state.brightness[0] == 100U);
    assert(LED_ControlApply(&state, BUTTON_UP_MASK) == 0U);
    assert(LED_ControlApply(&state, BUTTON_SELECT_MASK) != 0U);
    assert(state.selected_led == 1U && state.brightness[0] == 100U);
    for (unsigned i = 0; i < 5U; ++i)
        assert(LED_ControlApply(&state, BUTTON_DOWN_MASK) != 0U);
    assert(LED_ControlApply(&state, BUTTON_DOWN_MASK) == 0U);
    assert(LED_ControlApply(&state, BUTTON_UP_MASK) != 0U);
    assert(state.brightness[1] == 10U);
    assert(LED_ControlApply(&state, BUTTON_UP_MASK | BUTTON_DOWN_MASK) == 0U);
    LED_ControlApply(&state, BUTTON_SELECT_MASK);
    LED_ControlApply(&state, BUTTON_SELECT_MASK);
    assert(state.selected_led == 0U && state.brightness[0] == 100U);
    for (unsigned i = 0; i < 10U; ++i) LED_ControlApply(&state, BUTTON_DOWN_MASK);
    assert(state.brightness[0] == 0U);
    assert(LED_ControlApply(&state, BUTTON_DOWN_MASK) == 0U);
    state.brightness[0] = 95U;
    LED_ControlApply(&state, BUTTON_UP_MASK);
    assert(state.brightness[0] == 100U);
    state.brightness[0] = 5U;
    LED_ControlApply(&state, BUTTON_DOWN_MASK);
    assert(state.brightness[0] == 0U);
}

static void Test_ButtonDebounce(void)
{
    memset(&test_gpioa, 0, sizeof(test_gpioa));
    test_gpioa.IDR = 7U;
    Button_Init();
    assert((test_gpioa.CRL & 0xFFFU) == 0x888U && (test_gpioa.ODR & 7U) == 7U);
    for (unsigned i = 0; i < 4U; ++i) assert(Button_ScanPressed() == 0U);
    test_gpioa.IDR = 6U; /* Nhiễu nhấn/thả ngắn không tạo event. */
    assert(Button_ScanPressed() == 0U);
    test_gpioa.IDR = 7U;
    assert(Button_ScanPressed() == 0U);
    test_gpioa.IDR = 6U;
    for (unsigned i = 0; i < 3U; ++i) assert(Button_ScanPressed() == 0U);
    assert(Button_ScanPressed() == BUTTON_SELECT_MASK);
    for (unsigned i = 0; i < 20U; ++i) assert(Button_ScanPressed() == 0U);
    test_gpioa.IDR = 7U;
    for (unsigned i = 0; i < 4U; ++i) assert(Button_ScanPressed() == 0U);
    test_gpioa.IDR = 5U; /* PA1: sự kiện tăng. */
    for (unsigned i = 0; i < 3U; ++i) assert(Button_ScanPressed() == 0U);
    assert(Button_ScanPressed() == BUTTON_UP_MASK);
}

static void Test_PWM(void)
{
    memset(&test_tim1, 0, sizeof(test_tim1));
    test_gpioa.CRH = 0x44444444U;
    LED_PWM_Init();
    assert(test_tim1.PSC == 63U && test_tim1.ARR == 999U);
    assert(BOARD_CPU_HZ / (test_tim1.PSC + 1U) / (test_tim1.ARR + 1U) == 1000U);
    assert(test_tim1.CCER == 0x333U && test_tim1.BDTR == 0x8000U);
    assert(test_tim1.CCMR1 == 0x6868U && test_tim1.CCMR2 == 0x68U);
    assert(test_gpioa.CRH == 0x44444AAAU); /* PA13/PA14 được giữ nguyên. */
    LED_PWM_SetBrightness(0U, 0U);
    LED_PWM_SetBrightness(1U, 50U);
    LED_PWM_SetBrightness(2U, 100U);
    assert(test_tim1.CCR1 == 0U && test_tim1.CCR2 == 500U && test_tim1.CCR3 == 1000U);
    LED_PWM_SetBrightness(1U, 255U);
    assert(test_tim1.CCR2 == 1000U);
    LED_PWM_SetBrightness(3U, 0U);
    assert(test_tim1.CCR3 == 1000U);
}

static void Test_Blink(void)
{
    LED_BlinkState leds[3];
    const float frequencies[3] = {0.1f, 1.0f, 10.0f};
    const uint32_t half_periods[3] = {5000U, 500U, 50U};
    const uint32_t sample_times[] = {0U, 49U, 50U, 499U, 500U, 4999U, 5000U, 10000U};
    for (unsigned i = 0; i < 3U; ++i)
    {
        assert(LED_Blink_Init(&leds[i], frequencies[i], 1000U, 0U) != 0U);
        assert(leds[i].half_period_ticks == half_periods[i]);
    }
    /* Ba state dùng chung một hàm nhưng chuyển pha theo nhịp riêng. */
    for (unsigned sample = 0; sample < sizeof(sample_times) / sizeof(sample_times[0]); ++sample)
    {
        uint32_t now = sample_times[sample];
        for (unsigned i = 0; i < 3U; ++i)
        {
            uint32_t wait = LED_Blink_Update(&leds[i], now);
            assert(leds[i].is_on == (uint8_t)(1U ^ ((now / half_periods[i]) & 1U)));
            assert(wait == half_periods[i] - (now % half_periods[i]));
        }
    }
    /* Nhận message giữa chu kỳ không làm reset pha, không trôi nhịp khi task trễ. */
    assert(LED_Blink_Init(&leds[0], 1.0f, 1000U, 0U) != 0U);
    assert(LED_Blink_Update(&leds[0], 100U) == 400U);
    assert(LED_Blink_Update(&leds[0], 200U) == 300U);
    assert(LED_Blink_Update(&leds[0], 500U) == 500U && leds[0].is_on == 0U);
    assert(LED_Blink_Update(&leds[0], 2600U) == 400U && leds[0].is_on == 0U);
    assert(LED_Blink_Update(&leds[0], 3000U) == 500U && leds[0].is_on == 1U);

    assert(LED_Blink_Init(&leds[0], 10.0f, 1000U, UINT32_MAX - 24U) != 0U);
    assert(LED_Blink_Update(&leds[0], 25U) == 50U && leds[0].is_on == 0U);
    assert(LED_Blink_Update(&leds[0], 75U) == 50U && leds[0].is_on == 1U);
    assert(LED_Blink_Init(&leds[0], 0.0f, 1000U, 0U) != 0U);
    assert(LED_Blink_Update(&leds[0], 12345U) == UINT32_MAX && leds[0].is_on == 1U);
    assert(LED_Blink_Init(&leds[0], -1.0f, 1000U, 0U) == 0U);
    assert(LED_Blink_Init(&leds[0], 0.001f, 1000U, 0U) == 0U);
    assert(LED_Blink_Init(&leds[0], 101.0f, 1000U, 0U) == 0U);
    assert(LED_Blink_Init(&leds[0], NAN, 1000U, 0U) == 0U);
    assert(LED_Blink_Init(&leds[0], INFINITY, 1000U, 0U) == 0U);
    assert(LED_Blink_Init(&leds[0], 1.0f, 0U, 0U) == 0U);
    assert(LED_Blink_Init(&leds[0], 100.0f, 10U, 0U) == 0U);
}

static void ReleaseButton(void)
{
    test_gpioa.IDR |= 1U;
    for (unsigned i = 0; i < 3U; ++i)
    {
        release_callback(&timer_resets);
        assert((test_exti.IMR & 1U) == 0U);
    }
    release_callback(&timer_resets);
    assert((test_exti.IMR & 1U) != 0U);
}

static void Test_EXTI(void)
{
    memset(&test_exti, 0, sizeof(test_exti));
    test_gpioa.IDR = 1U;
    Button_EXTI_Init(&last_sent);
    assert(irq_priority == 5U);
    assert((test_exti.FTSR & 1U) != 0U && (test_exti.RTSR & 1U) == 0U);
    assert((test_exti.IMR & 1U) == 0U);
    Button_EXTI_Enable();
    test_exti.PR = 0U;
    EXTI0_IRQHandler();
    assert(button_press_count == 0U && sends == 0U);

    test_gpioa.IDR = 0U;
    test_exti.PR = 1U;
    EXTI0_IRQHandler();
    assert(button_press_count == 1U && last_sent == 1U && sends == 1U && yields == 1U);
    assert((test_exti.IMR & 1U) == 0U);
    for (unsigned i = 0; i < 10U; ++i) release_callback(&timer_resets);
    assert((test_exti.IMR & 1U) == 0U && button_press_count == 1U);
    ReleaseButton();

    test_gpioa.IDR = 0U;
    test_exti.PR = 1U;
    EXTI0_IRQHandler();
    assert(button_press_count == 2U && last_sent == 2U && yields == 2U);
    ReleaseButton();

    queue_result = pdFAIL;
    test_exti.PR = 1U;
    EXTI0_IRQHandler();
    assert(button_press_count == 3U && button_queue_dropped == 1U);
    assert(last_sent == 2U && sends == 2U);
    assert(button_timer_errors == 0U);
}

int main(void)
{
    Test_Control();
    Test_ButtonDebounce();
    Test_PWM();
    Test_Blink();
    Test_EXTI();
    puts("PASS: LED control, debounce, 1 kHz PWM, independent 0.1/1/10 Hz blink, tick wrap, EXTI queue/yield.");
    return 0;
}
