# Bài 02 — FreeRTOS: EXTI → Queue → UART1

Firmware STM32F103C8T6 dùng CMSIS và FreeRTOS V10.3.1 trong STM32CubeF1.
Một task ứng dụng UART nhận số đếm từ ISR EXTI0 và truyền ra máy tính.

## Nối mạch

| Chức năng | Chân STM32 | Kết nối |
|---|---|---|
| Nút nhấn EXTI0 | PA0 | Nút thường hở nối PA0 xuống GND khi nhấn |
| USART1 TX | PA9 | Nối RX của USB-UART mức logic 3.3 V |
| Ground | GND | Nối chung GND của STM32 và USB-UART |

PA0 dùng pull-up nội: thả HIGH, nhấn LOW. EXTI kích cạnh xuống.
UART1 chỉ cần TX; chân TX của adapter không cần nối vì firmware không nhận UART.
PA13/PA14 giữ cho ST-Link SWD.
Tháo LED khỏi PA9 khi chuyển từ Bài 1 sang bài này vì PA9 đang là UART TX.

Serial Terminal: 115200 baud, 8 data bits, no parity, 1 stop bit, không flow control.
Hercules, PuTTY hoặc terminal tương đương đều được.
HSI/2 x 16 tạo clock 64 MHz; BRR = 556, baud danh định xấp xỉ 115108.
Tần số thực tế còn phụ thuộc sai số HSI của chip.

## Hoạt động

Khởi động gửi banner và Count: 0.
Mỗi lần nhấn PA0:

1. EXTI0_IRQHandler xóa cờ ngắt và tăng button_press_count.
2. xQueueSendFromISR đưa bản sao số đếm vào queue.
3. portYIELD_FROM_ISR(higher_priority_task_woken) yêu cầu chuyển ngữ cảnh nếu có task ưu tiên cao hơn được đánh thức.
4. UART task đang Blocked ở xQueueReceive(..., portMAX_DELAY) nhận số đếm.
5. UART_SendCount định dạng số thập phân và gửi chuỗi ASCII qua USART1.

Ví dụ terminal:

```text
STM32F103 FreeRTOS EXTI -> Queue -> UART1
Count: 0
Count: 1
Count: 2
Count: 3
```

Không có delay hoặc in UART trong ISR.
EXTI0 có NVIC priority 5, phù hợp configMAX_SYSCALL_INTERRUPT_PRIORITY.
Queue được tạo trước EXTI; IRQ chỉ bật từ UART task sau khi scheduler đã chạy.
Nhờ vậy IRQ không gọi FreeRTOS API trước lúc kernel sẵn sàng.

## Chống dội và giữ nút

Sau cạnh xuống đầu tiên, EXTI0 tạm bị mask.
Software timer lấy mẫu trạng thái thả mỗi 10 ms; phải có 4 mẫu HIGH liên tiếp mới bật lại EXTI.
Giữ nút chỉ đếm một lần; thả rồi nhấn lại mới tăng số đếm.
Timer callback thuộc Timer Service task của kernel, không phải một UART task thứ hai.
Callback không block hoặc truyền UART.

Queue có 16 phần tử. Nếu queue đầy, button_queue_dropped tăng;
số đếm tổng vẫn tăng nhưng một số dòng UART có thể bị bỏ, không chặn ISR.
button_timer_errors giúp phát hiện lỗi gửi command tới timer queue.

## Cấu trúc

- src/main.c: khởi tạo board/app và chạy scheduler.
- src/app.c: tạo queue và UART task.
- src/button_exti.c: GPIO nút, EXTI0 ISR và chống dội.
- src/uart.c: driver UART, định dạng số đếm.
- inc/: header tương ứng.
- ../FreeRTOS_Common/: clock, GPIO, cấu hình kernel, hooks, linker, build.
- ../STM32CubeF1/: CMSIS/startup/FreeRTOS kernel.

## Build, test, flash

Từ workspace:

```text
make -C BT_2_FreeRTOS
make -C BT_2_FreeRTOS test
make -C BT_2_FreeRTOS flash
```

Kết quả: build/firmware.elf và build/firmware.bin.
Mỗi lần chỉ nạp một bài vào board. Lệnh flash bài này thay firmware Bài 1 trên board.

Test trên PC kiểm tra logic driver với thanh ghi và FreeRTOS API giả;
chưa thay thế kiểm tra scheduler, ngắt điện thực tế và UART trên STM32.
GNU ld có thể báo syscall stub nosys và RWX segment nhưng vẫn build được.

## Demo gợi ý

1. Mở terminal đúng cổng COM, 115200/8N1.
2. Reset board để thấy banner và Count: 0.
3. Nhấn/thả 10 lần; kiểm tra các dòng Count: 1 tới Count: 10.
4. Giữ nút khoảng 2 giây; cho thấy count không tăng liên tục.
5. Thả rồi nhấn lại; count tăng đúng một lần.
6. Quay video terminal và nút nhấn cùng lúc, dán link vào mục nộp bài.

## Debug

button_press_count: số lần nhấn; uart_last_count: giá trị mới nhất task UART đã nhận.
button_queue_dropped và button_timer_errors bình thường bằng 0.
board_fault_code/rtos_assert_file/rtos_assert_line giúp chẩn đoán kernel bị dừng.
Task UART priority 2; EXTI priority 5 là ưu tiên NVIC, hai loại priority này không cùng thang đo.

Tham khảo API chính thức:
[xQueueSendFromISR](https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/04-xQueueSendFromISR).

## Nộp bài

- Repository GitHub: [AnhTai-Cloud/Embedded_System](https://github.com/AnhTai-Cloud/Embedded_System).
- Source Bài 02: [BT_2_FreeRTOS](https://github.com/AnhTai-Cloud/Embedded_System/tree/main/BT_2_FreeRTOS).
- Video demo: **chưa điền**.

Giữ BT_2_FreeRTOS, FreeRTOS_Common và STM32CubeF1 cạnh nhau.
Xem [hướng dẫn clone và lấy submodule](../README.md#lấy-source-và-thư-viện) ở README gốc.
Cần thực hiện demo phần cứng và điền link video thật trước khi nộp.
