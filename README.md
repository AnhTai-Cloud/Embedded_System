# Embedded System — STM32F103 & FreeRTOS

Hai bài tập sử dụng **STM32F103C8T6**, **FreeRTOS V10.3.1**, CMSIS và ARM GNU Toolchain.
Code được tách thành module `.c/.h`; `main.c` chỉ khởi tạo board, ứng dụng và chạy scheduler.

| Bài tập | Nội dung | Tài liệu |
|---|---|---|
| **Bài 01** | Ba LED PWM 1 kHz, ba task LED độc lập; một task đọc ba nút để chọn LED, tăng/giảm độ sáng; truyền trạng thái bằng Queue. Có bổ sung nhịp nháy riêng 0.1/1/10 Hz qua một hàm cấu hình chung. | [BT_1_FreeRTOS](BT_1_FreeRTOS/README.md) |
| **Bài 02** | Nút nhấn tạo EXTI cạnh xuống; ISR đếm số lần nhấn, gửi Queue bằng `xQueueSendFromISR` và yêu cầu chuyển ngữ cảnh; một task UART chờ Queue rồi gửi số đếm tới máy tính. | [BT_2_FreeRTOS](BT_2_FreeRTOS/README.md) |

## Bài 01 — PWM, Task và Queue

**Yêu cầu:** cấu hình ba LED PWM 1 kHz và ba nút nhấn; mỗi LED có một task độc lập.
Một task nút nhấn chọn LED đang điều khiển, tăng/giảm độ sáng và gửi dữ liệu tới các task LED bằng Queue.

**Cách triển khai:**

- LED dương chung ở PA8/PA9/PA10, sử dụng TIM1 CH1/CH2/CH3; anode chung nối 3.3 V, mỗi LED có điện trở hạn dòng riêng.
- Nút PA0 chọn LED, PA1 tăng sáng, PA2 giảm sáng; các nút nhấn nối xuống GND và dùng pull-up nội.
- Mỗi LED lưu độ sáng riêng từ 0–100%, tăng/giảm 10% mỗi lần nhấn; có chống dội.
- Một hàm `LED_Task()` dùng chung được tạo thành ba task, mỗi task có context và Queue riêng.
- PWM luôn là 1 kHz. Phần mở rộng nhịp nháy mặc định là LED1 0.1 Hz, LED2 1 Hz, LED3 10 Hz; sửa tập trung trong `LED_Configure()` ở [app.c](BT_1_FreeRTOS/src/app.c).
- Đặt nhịp nháy `0.0f` để LED sáng liên tục theo độ sáng đã chọn; cả ba LED khởi động ở độ sáng PWM 50% trong pha sáng.

## Bài 02 — EXTI, Queue từ ISR và UART

**Yêu cầu:** cấu hình một nút EXTI cạnh xuống và UART1.
ISR tăng biến đếm, gửi số đếm vào Queue bằng `xQueueSendFromISR()`,
gọi `portYIELD_FROM_ISR()` khi cần chuyển ngữ cảnh.
Task UART ở trạng thái Blocked khi chờ Queue, nhận dữ liệu rồi định dạng và truyền tới Serial Terminal.

**Cách triển khai:**

- PA0 là input pull-up, nút nối PA0 xuống GND để tạo cạnh xuống.
- PA9 là USART1 TX, nối RX của USB-UART mức logic 3.3 V; hai thiết bị nối chung GND.
- Terminal dùng **115200 baud, 8N1**, không flow control.
- Có chống dội; giữ nút chỉ đếm một lần và phải thả rồi nhấn lại để tăng số đếm.

Ví dụ kết quả:

```text
STM32F103 FreeRTOS EXTI -> Queue -> UART1
Count: 0
Count: 1
Count: 2
Count: 3
```

## Cấu trúc repository

```text
Embedded_System/
├── BT_1_FreeRTOS/
│   ├── inc/                 Header app, nút, PWM, nhịp nháy, trạng thái LED
│   ├── src/                 Các module và main.c
│   ├── Makefile
│   └── README.md            Yêu cầu, nối mạch, hoạt động, demo
├── BT_2_FreeRTOS/
│   ├── inc/                 Header app, EXTI, UART
│   ├── src/                 Các module và main.c
│   ├── Makefile
│   └── README.md
├── FreeRTOS_Common/          Clock, GPIO, kernel config, linker, build rules, tests
├── STM32CubeF1/              Git submodule chính thức của STMicroelectronics
└── FreeRTOS_Exercises.md     Tổng quan hai bài
```

STM32CubeF1 được ghim tại commit `c7656f1fee178058be2f8ebfc8df9eaf91dc565e`.
Thư viện này chứa CMSIS và các submodule CMSIS Device/FreeRTOS được ghim theo phiên bản.
Giấy phép của mã bên thứ ba nằm trong chính dependency STM32CubeF1 và các submodule tương ứng.

## Lấy source và thư viện

Cách đầy đủ:

```sh
git clone --recurse-submodules https://github.com/AnhTai-Cloud/Embedded_System.git
cd Embedded_System
```

Nếu chỉ cần các dependency dùng trong hai bài, có thể lấy chọn lọc:

```sh
git clone https://github.com/AnhTai-Cloud/Embedded_System.git
cd Embedded_System
git submodule update --init STM32CubeF1
git -C STM32CubeF1 submodule update --init Drivers/CMSIS/Device/ST/STM32F1xx Middlewares/Third_Party/FreeRTOS
```

GitHub Download ZIP không tự tải nội dung submodule; nên dùng Git theo các bước trên.

## Build và kiểm thử

Cần ARM GNU Toolchain (`arm-none-eabi-gcc`, `objcopy`, `size`) và GNU Make.
Trên Windows, project đã dùng MSYS2 Make; test trên PC cần thêm GCC native.

Chạy từ thư mục gốc repository:

```sh
make -C BT_1_FreeRTOS
make -C BT_2_FreeRTOS
make -C BT_1_FreeRTOS test
```

Mỗi bài tạo `build/firmware.elf` và `build/firmware.bin`.
Các file build được bỏ qua bởi `.gitignore`.

Test trên PC kiểm tra chọn LED, giới hạn độ sáng, chống dội, ba nhịp nháy,
tick quay vòng, cấu hình PWM và luồng EXTI/Queue/yield với thanh ghi/API giả.
Đây là kiểm thử logic; cần chạy trên board để xác nhận điện áp, tần số và UART thực tế.

## Nạp bằng ST-Link

Mỗi lần nạp một bài:

```sh
make -C BT_1_FreeRTOS flash
```

Thay tên thư mục bằng `BT_2_FreeRTOS` khi nạp Bài 2.
Makefile có đường dẫn xPack OpenOCD mặc định cho máy phát triển.
Trên máy khác, ghi đè vị trí executable và scripts, ví dụ:

```sh
make -C BT_1_FreeRTOS flash OPENOCD=openocd OPENOCD_SCRIPTS=/usr/share/openocd/scripts
```

PA13/PA14 được giữ cho SWD. Khi chuyển từ Bài 01 sang Bài 02,
tháo LED khỏi PA9 để dùng chân này làm UART TX.

## Video demo

- Bài 01: chưa có link video.
- Bài 02: chưa có link video.

Kịch bản demo và các bước kiểm tra phần cứng nằm trong README của từng bài.
