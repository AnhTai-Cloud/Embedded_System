/* Các callback lỗi được kernel gọi khi bật các tùy chọn trong FreeRTOSConfig.h. */
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"

/* Giữ địa chỉ chuỗi tên file và số dòng của lần configASSERT thất bại.
 * volatile áp dụng cho biến con trỏ để debugger đọc được giá trị mới nhất.
 */
const char * volatile rtos_assert_file;
volatile uint32_t rtos_assert_line;

/* Được macro configASSERT gọi khi điều kiện không đúng. */
void RTOS_AssertFailed(const char *file, uint32_t line)
{
    rtos_assert_file = file;
    rtos_assert_line = line;
    Board_Panic(BOARD_FAULT_ASSERT);
}

/* heap_4 gọi hook này khi không đủ heap để đáp ứng yêu cầu cấp phát. */
void vApplicationMallocFailedHook(void)
{
    Board_Panic(BOARD_FAULT_ALLOC);
}

/* Kernel gọi khi phát hiện stack task bị tràn; không tiếp tục chạy task lỗi. */
void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    /* Hai tham số do kernel cung cấp; hiện mã lỗi chung đã đủ để dừng debug. */
    (void)task;
    (void)name;
    Board_Panic(BOARD_FAULT_STACK);
}
