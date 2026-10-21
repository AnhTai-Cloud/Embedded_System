#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "app.h"
#include "board.h"
#include "button.h"
#include "led_control.h"
#include "led_blink.h"
#include "led_pwm.h"

_Static_assert(LED_COUNT == CONTROL_LED_COUNT, "LED count mismatch");
_Static_assert(sizeof(TickType_t) == sizeof(uint32_t), "Blink requires 32-bit ticks");

typedef struct
{
    uint8_t brightness;
    uint8_t selected;
} LED_Message;

typedef struct
{
    uint8_t index;
    float blink_hz;
    QueueHandle_t queue;
} LED_TaskContext;

static LED_TaskContext led_tasks[LED_COUNT];

/* Có thể xem bằng debugger để biết LED đang chọn và duty cycle từng LED. */
volatile uint8_t app_selected_led;
volatile uint8_t app_brightness[LED_COUNT];
volatile uint8_t app_led_selected[LED_COUNT];
volatile uint8_t app_led_is_on[LED_COUNT];
volatile uint32_t app_led_updates[LED_COUNT];

/* ĐỔI NHỊP NHÁY TẠI MỘT CHỖ: không cần viết LED1_Task/LED2_Task/LED3_Task. */
static void LED_Configure(void)
{
    static const float frequency_hz[LED_COUNT] = {
        0.1f, /* LED1 PA8: sáng 5 giây, tắt 5 giây. */
        1.0f, /* LED2 PA9: sáng 500 ms, tắt 500 ms. */
        10.0f /* LED3 PA10: sáng 50 ms, tắt 50 ms. */
    };
    uint32_t led;
    for (led = 0U; led < LED_COUNT; ++led)
    {
        led_tasks[led].index = (uint8_t)led;
        led_tasks[led].blink_hz = frequency_hz[led];
    }
}

static void LED_Task(void *argument)
{
    LED_TaskContext *context = argument;
    LED_Message message;
    LED_BlinkState blink;
    uint8_t brightness = 0U;
    uint8_t applied_brightness = UINT8_MAX;
    uint8_t result = LED_Blink_Init(&blink, context->blink_hz,
                                   configTICK_RATE_HZ, xTaskGetTickCount());
    configASSERT(result != 0U);

    for (;;)
    {
        TickType_t timeout = (TickType_t)LED_Blink_Update(&blink, xTaskGetTickCount());
        uint8_t output = (blink.is_on != 0U) ? brightness : 0U;
        app_led_is_on[context->index] = blink.is_on;
        if (output != applied_brightness)
        {
            LED_PWM_SetBrightness(context->index, output);
            applied_brightness = output;
        }

        /* Chờ đồng thời message mới HOẶC đến hạn đảo pha; không delay cố định.
         * Vì thế LED1 nháy chậm vẫn nhận nút chỉnh sáng ngay, không phải đợi 5 s.
         */
        if (xQueueReceive(context->queue, &message, timeout) == pdPASS)
        {
            brightness = message.brightness;
            app_brightness[context->index] = message.brightness;
            app_led_selected[context->index] = message.selected;
            ++app_led_updates[context->index];
        }
    }
}

static void PublishState(const LED_ControlState *state)
{
    uint32_t led;
    app_selected_led = state->selected_led;
    for (led = 0U; led < LED_COUNT; ++led)
    {
        LED_Message message = {
            state->brightness[led],
            (uint8_t)(state->selected_led == led)
        };
        /* Queue dài 1: luôn giữ trạng thái mới nhất của riêng từng LED. */
        BaseType_t result = xQueueOverwrite(led_tasks[led].queue, &message);
        configASSERT(result == pdPASS);
    }
}

static void Button_Task(void *argument)
{
    LED_ControlState state;
    TickType_t wake_time = xTaskGetTickCount();
    (void)argument;
    LED_ControlInit(&state);
    PublishState(&state);
    for (;;)
    {
        vTaskDelayUntil(&wake_time, pdMS_TO_TICKS(BUTTON_SCAN_MS));
        if (LED_ControlApply(&state, Button_ScanPressed()) != 0U)
        {
            PublishState(&state);
        }
    }
}

void App_Init(void)
{
    uint32_t led;
    static const char * const task_names[LED_COUNT] = {"LED1", "LED2", "LED3"};
    Button_Init();
    LED_PWM_Init();
    LED_Configure();

    /* Mỗi LED có queue riêng: một queue chung không broadcast tới ba task. */
    for (led = 0U; led < LED_COUNT; ++led)
    {
        led_tasks[led].queue = xQueueCreate(1U, sizeof(LED_Message));
        if (led_tasks[led].queue == NULL ||
            xTaskCreate(LED_Task, task_names[led], 192U, &led_tasks[led],
                        2U, NULL) != pdPASS)
        {
            Board_Panic(BOARD_FAULT_CREATE);
        }
    }
    if (xTaskCreate(Button_Task, "Buttons", 256U, NULL, 2U, NULL) != pdPASS)
    {
        Board_Panic(BOARD_FAULT_CREATE);
    }
}
