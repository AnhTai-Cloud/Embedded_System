# Bài 01 — FreeRTOS: điều khiển độ sáng 3 LED qua Queue

Firmware STM32F103C8T6 dùng CMSIS và FreeRTOS V10.3.1 có sẵn trong STM32CubeF1.
Project dùng GNU Make + ARM GCC như các bài trước. Cấu hình LED dương chung.

## Nối mạch

| Chức năng | Chân STM32 | Kết nối |
|---|---|---|
| LED1, TIM1_CH1 | PA8 | Cathode LED1 qua điện trở hạn dòng |
| LED2, TIM1_CH2 | PA9 | Cathode LED2 qua điện trở hạn dòng |
| LED3, TIM1_CH3 | PA10 | Cathode LED3 qua điện trở hạn dòng |
| Nút chuyển LED | PA0 | Nút thường hở nối PA0 với GND khi nhấn |
| Nút tăng độ sáng | PA1 | Nút thường hở nối PA1 với GND khi nhấn |
| Nút giảm độ sáng | PA2 | Nút thường hở nối PA2 với GND khi nhấn |

Anode chung nối 3.3 V; mỗi LED có điện trở hạn dòng riêng.
Ba nút dùng pull-up nội: thả = HIGH, nhấn = LOW.
Với nút 4 chân, chọn hai chân thuộc hai tiếp điểm khác nhau; dùng đồng hồ kiểm tra nếu chưa rõ cặp chân.
PA13/PA14 giữ cho ST-Link SWD. PA12/PB12/PB13/PA15 không dùng trong bài này.

## Hoạt động

- Khởi động: chọn LED1; cả ba LED ở độ sáng PWM 50% trong pha sáng để quan sát được ba nhịp nháy.
- LED1 nháy 0.1 Hz (sáng/tắt mỗi 5 giây), LED2 1 Hz (500 ms), LED3 10 Hz (50 ms).
- Nhấn PA0 chuyển LED1 → LED2 → LED3 → LED1.
- Nhấn PA1 tăng LED đang chọn 10%; nhấn PA2 giảm 10%.
- Giới hạn 0–100%, không tràn số.
- Chuyển LED giữ nguyên độ sáng đã đặt cho từng LED.
- Mỗi lần nhấn chỉ tạo một sự kiện; giữ nút không tự lặp.
- Nút được lấy mẫu 10 ms và cần 4 mẫu liên tiếp ổn định (khoảng 30–40 ms).
- Nhấn tăng và giảm cùng lúc: không đổi độ sáng.

PWM phần cứng TIM1 chạy độc lập với scheduler:
64 MHz / (PSC + 1) / (ARR + 1) = 64 MHz / 64 / 1000 = 1 kHz.
Polarity LOW phù hợp LED dương chung: duty 0% luôn HIGH/tắt, duty 100% luôn LOW/sáng.
CCR được preload và cập nhật vào chu kỳ PWM kế tiếp.

## Chỉ sửa một hàm để đổi nhịp nháy

Sửa bảng tần số trong hàm LED_Configure() ở src/app.c:

```c
static const float frequency_hz[LED_COUNT] = {
    0.1f,  /* LED1 */
    1.0f,  /* LED2 */
    10.0f  /* LED3 */
};
```

Đơn vị Hz, hỗ trợ 0.01–100 Hz; đặt 0.0f để LED không nháy.
Một hàm LED_Task(void *argument) dùng chung được tạo thành ba task,
mỗi task nhận index, tần số và queue của riêng nó qua context.
Không cần viết ba hàm xử lý LED riêng. Tính nhịp nằm trong module led_blink.c/.h.

Nhịp nháy này bật/tắt độ sáng PWM 1 kHz: pha sáng áp dụng độ sáng đã chọn,
pha tắt áp dụng 0%. Nút tăng/giảm vẫn chỉnh độ sáng của LED đang chọn.
Đổi độ sáng trong pha tắt được lưu ngay và thấy ở pha sáng tiếp theo.
Đây là nhịp nháy riêng từng LED; PWM TIM1 vẫn luôn 1 kHz cho cả ba kênh.

## Task và Queue

Có 4 task ứng dụng: Buttons, LED1, LED2, LED3.
Task Buttons đọc nút, chống dội, cập nhật lựa chọn và độ sáng rồi gửi trạng thái tới 3 queue riêng.
Mỗi LED task nhận bằng xQueueReceive với timeout tới lần chuyển pha nháy tiếp theo.
Message mới đánh thức task ngay, không bị kẹt chờ 5 giây ở LED1.
Nếu hết timeout, task đổi pha sáng/tắt và tính lại thời gian chờ.
Task ở trạng thái Blocked giữa các sự kiện; timer vẫn phát PWM 1 kHz.
Nhận Queue không reset nhịp nháy; phép tính tick xử lý cả trường hợp counter uint32_t quay vòng.

Mỗi queue dài 1 phần tử, chứa độ sáng và trạng thái selected của LED đó.
xQueueOverwrite giữ trạng thái mới nhất, không tích lũy lệnh cũ khi người dùng nhấn nhanh.
Ba queue tránh tình trạng một queue chung chỉ giao message cho một trong ba task.
FreeRTOS còn có Idle task và Timer Service task hệ thống; chúng không thay thế các task ứng dụng yêu cầu.

## Cấu trúc

- src/main.c: khởi tạo board/app và chạy scheduler.
- src/app.c: tạo task/queue, điều phối message.
- src/led_control.c: trạng thái lựa chọn và độ sáng.
- src/led_blink.c: hàm nháy dùng chung, đổi Hz sang tick và quản lý pha từng LED.
- src/led_pwm.c: driver TIM1 PWM.
- src/button.c: GPIO nút và chống dội.
- inc/: header tương ứng.
- ../FreeRTOS_Common/: clock, GPIO, FreeRTOSConfig, hooks lỗi, linker và build rules.
- ../STM32CubeF1/: CMSIS, startup và FreeRTOS kernel. Phải giữ thư mục này cạnh project.

## Build, test, flash

Từ thư mục gốc workspace:

```text
make -C BT_1_FreeRTOS
make -C BT_1_FreeRTOS test
make -C BT_1_FreeRTOS flash
```

Hoặc vào BT_1_FreeRTOS và dùng make / make flash.
ELF/BIN nằm trong build/. Không nạp Bài 2 trong lúc đang demo Bài 1 vì hai firmware dùng chung board.

Test trên PC kiểm tra logic độ sáng/lựa chọn, ba nhịp 0.1/1/10 Hz, tick quay vòng,
chống dội, cấu hình PWM và logic EXTI của Bài 2 với API/thanh ghi giả.
Test này chưa chứng minh tần số điện áp thực tế hay scheduler trên STM32; cần đo/quan sát board sau khi flash.
GNU ld có thể báo syscall stub nosys và segment RWX như toolchain các bài trước; build vẫn tạo ELF/BIN.

## Demo gợi ý

1. Khởi động, quan sát ba LED nháy với nhịp riêng, độ sáng trong pha sáng là 50%.
   LED1 có chu kỳ dài 10 giây nên quan sát ít nhất 20 giây.
2. Tăng LED1 tới 100%, giảm tới 0%; kiểm tra giới hạn không bị tràn.
3. Chuyển LED2, đặt 30%; chuyển LED3, đặt 70%.
4. Quay lại LED1/LED2 và cho thấy độ sáng từng LED được giữ.
5. Nếu có oscilloscope/logic analyzer, đo PA8/PA9/PA10 ở duty khác 0/100 để xác nhận 1 kHz.
6. Quay video và dán link thật vào mục nộp bài bên dưới.

## Debug

app_selected_led: 0/1/2; app_brightness[3]: duty đã áp dụng.
app_led_selected[3] và app_led_updates[3] cho biết message đã tới từng task.
app_led_is_on[3] cho biết pha sáng/tắt hiện tại; app_brightness[3] vẫn là độ sáng đã đặt.
board_fault_code = 0 khi chưa ghi nhận lỗi; tra enum BoardFault ở board.h nếu dừng.
rtos_assert_file/rtos_assert_line cho biết vị trí configASSERT.
Stack task tính theo word 32 bit; heap RTOS 10 KiB trong RAM 20 KiB.

## Nộp bài

- Repository GitHub: [AnhTai-Cloud/Embedded_System](https://github.com/AnhTai-Cloud/Embedded_System).
- Source Bài 01: [BT_1_FreeRTOS](https://github.com/AnhTai-Cloud/Embedded_System/tree/main/BT_1_FreeRTOS).
- Video demo: **chưa điền**.

Giữ BT_1_FreeRTOS, FreeRTOS_Common và dependency STM32CubeF1 vào cùng bố cục.
Xem [hướng dẫn clone và lấy submodule](../README.md#lấy-source-và-thư-viện) ở README gốc.
Firmware đã build; cần quay demo phần cứng và điền link video thật trước khi nộp.
