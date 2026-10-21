#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"
#include "board.h"
#include "gpio.h"
#include "button_exti.h"

#define BUTTON_LINE (1UL << 0)
#define RELEASE_SAMPLE_MS 10U
#define RELEASE_STABLE_SAMPLES 4U

static QueueHandle_t count_queue;
static TimerHandle_t release_timer;
static volatile uint8_t release_samples;
volatile uint32_t button_press_count;
volatile uint32_t button_queue_dropped;
volatile uint32_t button_timer_errors;

/*
 * Sau khi nhận một nhấn: mask EXTI tới lúc nút được thả ổn định 30 ms.
 * Timer callback không delay, không truyền UART, không gọi API có thể block.
 */
static void Button_ReleaseTimer(TimerHandle_t timer)
{
    if (GPIO_IsLow(GPIOA, 0U) == 0U)
    {
        ++release_samples;
    }
    else
    {
        release_samples = 0U;
    }
    if (release_samples >= RELEASE_STABLE_SAMPLES)
    {
        EXTI->PR = BUTTON_LINE;
        NVIC_ClearPendingIRQ(EXTI0_IRQn);
        EXTI->IMR |= BUTTON_LINE;
    }
    else
    {
        BaseType_t result = xTimerReset(timer, 0U);
        configASSERT(result == pdPASS);
    }
}

void Button_EXTI_Init(QueueHandle_t queue)
{
    count_queue = queue;
    configASSERT(count_queue != NULL);
    GPIO_InputPullUp(GPIOA, 0U);
    AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0; /* EXTI0 lấy tín hiệu từ PA0. */
    EXTI->IMR &= ~BUTTON_LINE;
    EXTI->RTSR &= ~BUTTON_LINE;
    EXTI->FTSR |= BUTTON_LINE; /* Nhấn nối GND: HIGH -> LOW, falling edge. */
    EXTI->PR = BUTTON_LINE;
    NVIC_SetPriority(EXTI0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    release_timer = xTimerCreate("Release", pdMS_TO_TICKS(RELEASE_SAMPLE_MS),
                                pdFALSE, NULL, Button_ReleaseTimer);
    if (release_timer == NULL)
    {
        Board_Panic(BOARD_FAULT_CREATE);
    }
}

void Button_EXTI_Enable(void)
{
    /* Chỉ gọi trong UART task: FreeRTOS tick/queue/timer đã hoạt động. */
    EXTI->PR = BUTTON_LINE;
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    EXTI->IMR |= BUTTON_LINE;
    NVIC_EnableIRQ(EXTI0_IRQn);
}

void EXTI0_IRQHandler(void)
{
    BaseType_t higher_priority_task_woken = pdFALSE;
    uint32_t count;
    if ((EXTI->PR & BUTTON_LINE) == 0U)
    {
        return;
    }
    EXTI->PR = BUTTON_LINE;
    EXTI->IMR &= ~BUTTON_LINE;
    release_samples = 0U;
    ++button_press_count;
    count = button_press_count;

    /* Gửi số đếm từ ISR, không in UART ngay trong ngắt. */
    if (xQueueSendFromISR(count_queue, &count,
                         &higher_priority_task_woken) != pdPASS)
    {
        ++button_queue_dropped;
    }
    if (xTimerResetFromISR(release_timer, &higher_priority_task_woken) != pdPASS)
    {
        /* Theo dõi lỗi và tránh khóa nút mãi nếu timer queue bị đầy. */
        ++button_timer_errors;
        EXTI->IMR |= BUTTON_LINE;
    }
    /* Nếu task nhận có ưu tiên cao hơn, yêu cầu chuyển ngữ cảnh ngay. */
    portYIELD_FROM_ISR(higher_priority_task_woken);
}
