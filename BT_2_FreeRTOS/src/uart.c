/*
 * Driver USART1 truyền một chiều trên PA9, định dạng 115200 baud, 8N1.
 * Task gọi driver để gửi chuỗi; ISR EXTI chỉ gửi Queue và không truyền UART.
 */
#include "board.h"
#include "gpio.h"
#include "uart.h"

/* Board_Init phải chạy trước để PCLK2 đạt 64 MHz như BOARD_CPU_HZ. */
void UART_Init(void)
{
    /* Cấp clock ngoại vi trước khi ghi thanh ghi cấu hình của USART1. */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    /* 0xA: MODE=10 (2 MHz), CNF=10 (alternate function push-pull). */
    GPIO_ConfigPin(GPIOA, 9U, 0xAUL); /* USART1_TX, AF push-pull. */
    /* Tắt USART trong lúc cấu hình. Giá trị reset chọn 8 bit, không parity,
     * một stop bit, không flow control; chỉ bật bộ truyền TE ở cuối hàm.
     */
    USART1->CR1 = 0U;
    USART1->CR2 = 0U;
    USART1->CR3 = 0U;
    /* Oversampling 16: BRR xấp xỉ PCLK2/baud. Cộng nửa mẫu số để làm tròn;
     * với 64 MHz và 115200, BRR=556, baud danh định xấp xỉ 115108.
     */
    USART1->BRR = (BOARD_CPU_HZ + UART_BAUD / 2UL) / UART_BAUD;
    /* UE bật USART, TE cho phép truyền. Không dùng RX nên không bật RE. */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE;
}

/* Hàm nội bộ gửi một byte ASCII, chỉ được dùng trong module UART. */
static void UART_SendChar(char value)
{
    /* TXE=1: thanh ghi dữ liệu truyền trống, có thể nạp byte tiếp theo.
     * Đây là thăm dò trong task, không phải trạng thái Blocked của Queue;
     * các ngắt và task có ưu tiên cao hơn vẫn có thể được xử lý bình thường.
     */
    while ((USART1->SR & USART_SR_TXE) == 0U)
    {
    }
    /* Ghi DR để phần cứng chuyển byte sang khung UART và phát trên PA9. */
    USART1->DR = (uint8_t)value;
}

/* Gửi từng ký tự tới khi gặp ký tự kết thúc chuỗi; không cần truyền độ dài. */
void UART_SendString(const char *text)
{
    while (*text != '\0')
    {
        UART_SendChar(*text++);
    }
}

/* Chuyển uint32_t thành số thập phân mà không cần printf hoặc cấp phát động. */
void UART_SendCount(uint32_t count)
{
    char digits[10]; /* uint32_t lớn nhất 4294967295: đúng 10 chữ số. */
    uint32_t length = 0U;
    UART_SendString("Count: ");
    /* Lấy phần dư chia 10 từ hàng đơn vị lên, nên digits chứa thứ tự ngược.
     * do-while bảo đảm giá trị 0 vẫn tạo được một ký tự '0'.
     */
    do
    {
        digits[length++] = (char)('0' + count % 10UL);
        count /= 10UL;
    } while (count != 0U);
    /* Đọc ngược mảng để gửi chữ số lớn trước. Ví dụ 123 lưu 3,2,1 rồi gửi 1,2,3. */
    while (length != 0U)
    {
        UART_SendChar(digits[--length]);
    }
    /* CR đưa con trỏ về đầu dòng, LF xuống dòng mới trên Serial Terminal. */
    UART_SendString("\r\n");
}
