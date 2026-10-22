/*
 * Nút nhấn PA0: thả = HIGH nhờ pull-up nội, nhấn nối GND = LOW.
 * Cạnh xuống gọi ISR để đếm và gửi Queue. Chống dội bằng cách tạm khóa
 * EXTI0, chờ thả ổn định qua software timer rồi mở lại ngắt.
 */
#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"
#include "board.h"
#include "gpio.h"
#include "button_exti.h"

#define BUTTON_LINE (1UL << 0)      /* Mặt nạ bit của đường ngắt EXTI0. */
#define RELEASE_SAMPLE_MS 10U       /* Khoảng cách giữa hai lần kiểm tra thả nút. */
#define RELEASE_STABLE_SAMPLES 4U   /* Cần 4 mẫu HIGH liên tiếp, khoảng 30-40 ms. */

/* Handle Queue do App_Init tạo, dùng để gửi số đếm sang UART_Task. */
static QueueHandle_t count_queue;
/* Timer một lần; callback tự khởi động lại khi chưa được phép mở EXTI0. */
static TimerHandle_t release_timer;
/* Số mẫu thả liên tiếp; ISR đặt về 0, Timer Service task cập nhật khi kiểm tra. */
static volatile uint8_t release_samples;
/* Biến đếm được khởi tạo bằng 0 trong vùng .bss khi vi điều khiển khởi động. */
volatile uint32_t button_press_count;
/* Hai biến lỗi giúp phân biệt mất dữ liệu Queue với lỗi lệnh timer khi debug. */
volatile uint32_t button_queue_dropped;
volatile uint32_t button_timer_errors;

/*
 * Callback chạy trong Timer Service task của FreeRTOS, không chạy trong ISR.
 * Mỗi lần kiểm tra cách nhau 10 ms; chỉ mở ngắt khi đủ 4 mẫu HIGH liên tiếp.
 * Callback không delay, không truyền UART và không chờ Queue để tránh giữ
 * Timer Service task quá lâu, ảnh hưởng các software timer khác.
 */
static void Button_ReleaseTimer(TimerHandle_t timer)
{
    if (GPIO_IsLow(GPIOA, 0U) == 0U)
    {
        /* Hàm IsLow trả 0 nghĩa là chân đang HIGH, tương ứng nút đã được thả. */
        ++release_samples;
    }
    else
    {
        /* Còn giữ nút hoặc tín hiệu lại LOW do dội: đếm lại chuỗi mẫu thả. */
        release_samples = 0U;
    }
    if (release_samples >= RELEASE_STABLE_SAMPLES)
    {
        /* PR là thanh ghi ghi 1 để xóa cờ. Loại các cạnh dội đã tích lũy
         * trong lúc khóa, xóa pending ở NVIC rồi cho phép lần nhấn mới.
         */
        EXTI->PR = BUTTON_LINE;
        NVIC_ClearPendingIRQ(EXTI0_IRQn);
        EXTI->IMR |= BUTTON_LINE;
    }
    else
    {
        /* Timer một lần đã hết hạn: gửi lệnh chạy lại sau 10 ms.
         * Thời gian chờ 0 tick vì đang ở chính Timer Service task.
         */
        BaseType_t result = xTimerReset(timer, 0U);
        configASSERT(result == pdPASS);
    }
}

/* Cấu hình phần cứng và tạo timer; chưa bật IRQ để tránh chạy trước scheduler. */
void Button_EXTI_Init(QueueHandle_t queue)
{
    count_queue = queue;
    configASSERT(count_queue != NULL);
    /* Chọn input pull-up nội; không cần điện trở kéo ngoài cho nút PA0. */
    GPIO_InputPullUp(GPIOA, 0U);
    AFIO->EXTICR[0] &= ~AFIO_EXTICR1_EXTI0; /* EXTI0 lấy tín hiệu từ PA0. */
    /* IMR=0 tạm khóa đường ngắt; RTSR=0 bỏ cạnh lên; FTSR=1 nhận cạnh xuống. */
    EXTI->IMR &= ~BUTTON_LINE;
    EXTI->RTSR &= ~BUTTON_LINE;
    EXTI->FTSR |= BUTTON_LINE; /* Nhấn nối GND: HIGH -> LOW, falling edge. */
    EXTI->PR = BUTTON_LINE;
    /* Đây là ưu tiên NVIC, không phải ưu tiên task UART.
     * Giá trị 5 phù hợp giới hạn cho ISR gọi API FreeRTOS có hậu tố FromISR.
     */
    NVIC_SetPriority(EXTI0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    /* pdFALSE chọn timer một lần. Callback sẽ tự khởi động lại nếu nút chưa thả.
     * Handle timer được giữ lại để ISR gửi lệnh chạy timer khi có lần nhấn.
     */
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
    /* Xóa yêu cầu cũ trước, sau đó mở cả EXTI0 và ngắt tương ứng trong NVIC. */
    EXTI->PR = BUTTON_LINE;
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    EXTI->IMR |= BUTTON_LINE;
    NVIC_EnableIRQ(EXTI0_IRQn);
}

/* Phần cứng gọi hàm này khi EXTI0 phát sinh; không gọi trực tiếp từ task. */
void EXTI0_IRQHandler(void)
{
    /* API FromISR có thể đặt cờ này thành pdTRUE khi đánh thức task ưu tiên cao hơn. */
    BaseType_t higher_priority_task_woken = pdFALSE;
    uint32_t count;
    if ((EXTI->PR & BUTTON_LINE) == 0U)
    {
        /* Không có cờ EXTI0 thì không xử lý và không tăng số đếm. */
        return;
    }
    /* Xác nhận ngắt rồi khóa EXTI0 ngay, tránh đếm nhiều cạnh do nút cơ khí dội. */
    EXTI->PR = BUTTON_LINE;
    EXTI->IMR &= ~BUTTON_LINE;
    release_samples = 0U;
    ++button_press_count;
    /* Dùng bản sao cục bộ. Queue sẽ chép giá trị trước khi ISR kết thúc,
     * vì vậy không lưu lại địa chỉ biến trên stack này để sử dụng về sau.
     */
    count = button_press_count;

    /* Gửi bằng API dành riêng cho ISR. Không gọi xQueueSend thường vì ISR
     * không được chờ. Nếu Queue đầy thì ghi nhận lỗi, số đếm tổng vẫn tăng.
     */
    if (xQueueSendFromISR(count_queue, &count,
                         &higher_priority_task_woken) != pdPASS)
    {
        ++button_queue_dropped;
    }
    /* Lệnh này đưa yêu cầu chạy timer vào hàng đợi lệnh của Timer Service task.
     * Dùng chung cờ woken để giữ lại yêu cầu chuyển ngữ cảnh từ cả hai API.
     */
    if (xTimerResetFromISR(release_timer, &higher_priority_task_woken) != pdPASS)
    {
        /* Theo dõi lỗi và tránh khóa nút mãi nếu timer queue bị đầy. */
        ++button_timer_errors;
        EXTI->IMR |= BUTTON_LINE;
    }
    /* Yêu cầu PendSV nếu cần, để scheduler chọn task Ready ưu tiên cao nhất
     * khi thoát ngắt. Đây không phải lời gọi trực tiếp UART_Task;
     * Timer Service task ưu tiên 3 có thể chạy trước task UART ưu tiên 2.
     */
    portYIELD_FROM_ISR(higher_priority_task_woken);
}
