# Hai bài STM32F103 + FreeRTOS

| Bài | Project | Phần cứng và chức năng |
|---|---|---|
| 01 | BT_1_FreeRTOS | LED dương chung PA8/PA9/PA10, PWM 1 kHz, nhịp nháy riêng 0.1/1/10 Hz; nút PA0 chọn, PA1 tăng, PA2 giảm |
| 02 | BT_2_FreeRTOS | Nút PA0/EXTI cạnh xuống; đếm trong ISR, Queue, một task UART1 TX PA9 115200/8N1 |

Hai bài là hai firmware độc lập; nạp một bài mỗi lần.
Nút nhấn ở các bài FreeRTOS nối input xuống GND và dùng pull-up nội.
Đây là cách nối khác với input pull-down của BT_3.

Code được tách inc/src; mỗi main.c chỉ khởi tạo và chạy scheduler.
Phần dùng chung ở FreeRTOS_Common, dependency CMSIS/FreeRTOS ở STM32CubeF1.

## Lệnh từ workspace

```text
make -C BT_1_FreeRTOS
make -C BT_2_FreeRTOS
make -C BT_1_FreeRTOS test
```

Nạp bài cần demo:

```text
make -C BT_1_FreeRTOS flash
```

Hoặc thay BT_1_FreeRTOS bằng BT_2_FreeRTOS.
Chi tiết nối mạch, chạy thử, debug và mục link GitHub/video ở README của từng bài.
