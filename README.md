# Hệ thống Giám sát Nông nghiệp Thông minh 🌱

[![STM32](https://img.shields.io/badge/STM32-Microcontroller-blue.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32-32-bit-arm-cortex-mcus.html)
[![ESP32](https://img.shields.io/badge/ESP32-WiFi%20Module-red.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Platform](https://img.shields.io/badge/Platform-STM32CubeIDE%20%7C%20Arduino-orange)](https://www.st.com/en/development-tools/stm32cubeide.html)

> **Hệ thống giám sát nông nghiệp thông minh sử dụng IoT để thu thập dữ liệu, giám sát từ xa và điều khiển tưới tiêu, tối ưu hóa quá trình canh tác.**

**[▶️ Xem Video Demo Hoạt Động](https://youtu.be/CVzV6Cb0qsQ?feature=shared)**

## 📋 Tổng quan
Dự án "Hệ thống Giám sát Nông nghiệp Thông minh" được phát triển nhằm giải quyết các thách thức trong nông nghiệp truyền thống bằng cách ứng dụng công nghệ IoT và tự động hóa. Hệ thống cho phép người dùng:

-   📊 **Thu thập dữ liệu**: Giám sát liên tục và chính xác các thông số môi trường quan trọng như độ ẩm đất, nhiệt độ, độ ẩm không khí, ánh sáng và mưa.
-   🌐 **Giám sát từ xa**: Hiển thị dữ liệu theo thời gian thực qua một giao diện web trực quan, có thể truy cập từ bất kỳ thiết bị nào trong cùng mạng.
-   💧 **Điều khiển thông minh**: Tự động hoặc thủ công bật/tắt hệ thống tưới tiêu từ xa, đảm bảo cây trồng luôn được chăm sóc tối ưu và tiết kiệm tài nguyên.
-   ☁️ **Lưu trữ đám mây**: Đồng bộ dữ liệu lên nền tảng ThingSpeak để lưu trữ, phân tích và theo dõi xu hướng dài hạn.

## 🏗️ Kiến trúc Hệ thống
Hệ thống được thiết kế theo kiến trúc module, bao gồm khối thu thập dữ liệu, khối xử lý trung tâm và khối giao tiếp, điều khiển. Dữ liệu được truyền tuần tự từ cảm biến đến người dùng và lệnh điều khiển được gửi theo chiều ngược lại.

![Sơ đồ luồng hoạt động của hệ thống](https://github.com/toandz20cm/Smart-Farm/raw/main/workflow-diagram.png)

## 🛠️ Công nghệ Sử dụng

| Lĩnh vực | Công nghệ | Chi tiết |
| :--- | :--- | :--- |
| **Phần cứng** | **Vi điều khiển** | STM32F103 (Black Pill) - Lõi ARM Cortex-M3. |
| | **Module WiFi** | ESP32-WROOM-32 - Dual-core, 240MHz, tích hợp Wi-Fi & Bluetooth. |
| | **Cảm biến** | - Độ ẩm đất (Điện dung)<br>- DHT11 (Nhiệt độ & Độ ẩm không khí)<br>- Quang trở LDR (Ánh sáng)<br>- Cảm biến mưa |
| | **Thiết bị ngoại vi** | Relay 5V, máy bơm nước mini. |
| **Phần mềm** | **Firmware STM32** | Lập trình C/C++ trên **STM32CubeIDE**. |
| | **Firmware ESP32** | Lập trình C++ trên **Arduino IDE**. |
| | **Giao diện Web** | HTML, CSS, JavaScript bất đồng bộ (AJAX). |
| | **Xử lý tín hiệu** | Bộ lọc Trung bình trượt (Moving Average), Trung vị (Median Filter). |
| **Giao tiếp** | **Giữa vi điều khiển** | **UART** (Tốc độ 9600 bps, 8N1). |
| | **Web & Cloud** | HTTP, API REST. |


## ✨ Tính năng Chính

-   **Dashboard thời gian thực**: Giao diện web hiển thị 5 thông số môi trường, tự động cập nhật mỗi giây mà không cần tải lại trang.
-   **Điều khiển bơm linh hoạt**: Chế độ `Thủ công` (bật/tắt ngay lập tức) và `Tự động` (dựa trên ngưỡng độ ẩm đất).
-   **Lọc nhiễu tín hiệu**: Áp dụng các thuật toán lọc số để loại bỏ nhiễu từ cảm biến analog, đảm bảo dữ liệu chính xác và ổn định.
-   **Phản hồi trạng thái tức thì**: Hệ thống gửi thông báo xác nhận thành công mỗi khi lệnh điều khiển bơm được thực thi.
-   **Kết nối bền bỉ**: Tự động kết nối lại Wi-Fi khi mất mạng, đảm bảo hệ thống luôn sẵn sàng.

## 🔌 Sơ đồ Kết nối và Cài đặt

### Sơ đồ nguyên lý
![Sơ đồ kết nối phần cứng](https://github.com/toandz20cm/Smart-Farm/raw/main/pin-circuit.png)

### Cấu hình chân (Pinout)

#### 1. STM32 Black Pill
| Chân | Chức năng | Kết nối tới | Ghi chú |
| :--- | :--- | :--- | :--- |
| **PA0** | ADC | Cảm biến Độ ẩm đất (A0) | Đọc giá trị analog |
| **PA1** | ADC | Cảm biến Ánh sáng (A0) | Đọc giá trị analog |
| **PA4** | ADC | Cảm biến Mưa (A0) | Đọc giá trị analog |
| **PA5** | GPIO_Output | Relay (IN) | Điều khiển bật/tắt bơm |
| **PA7** | GPIO_Input | Cảm biến DHT11 (DATA) | Đọc dữ liệu số |
| **PA9** | UART1_TX | ESP32 (RXD) | Gửi dữ liệu cảm biến |
| **PA10**| UART1_RX | ESP32 (TXD) | Nhận lệnh điều khiển |
| **5V/GND** | Nguồn | Các module | Cấp nguồn chung |

#### 2. ESP32 Dev Kit
| Chân | Chức năng | Kết nối tới | Ghi chú |
| :--- | :--- | :--- | :--- |
| **GPIO17 (TXD)** | UART2_TX | STM32 (PA10) | Gửi lệnh điều khiển |
| **GPIO16 (RXD)** | UART2_RX | STM32 (PA9) | Nhận dữ liệu cảm biến |
| **VIN/GND** | Nguồn | Nguồn 5V | Cấp nguồn cho ESP32 |

### Các bước cài đặt
1.  **Clone repository về máy:**
    ```bash
    git clone https://github.com/toandz20cm/Smart-Farm.git
    ```

2.  **Nạp code cho STM32:**
    - Mở project trong thư mục `STM32_Code` bằng **STM32CubeIDE**.
    - Kết nối board STM32 với máy tính qua bộ nạp **ST-Link**.
    - Biên dịch và nạp code cho vi điều khiển.

3.  **Nạp code cho ESP32:**
    - Mở file `ESP32_Code/ESP32_Code.ino` bằng **Arduino IDE**.
    - Cài đặt board **ESP32** trong `Boards Manager`.
    - Cập nhật thông tin Wi-Fi và API Key của ThingSpeak trong file code:
      ```cpp
      const char* ssid = "YOUR_WIFI_SSID";
      const char* password = "YOUR_WIFI_PASSWORD";
      String apiKey = "YOUR_THINGSPEAK_API_KEY";
      ```
    - Kết nối board ESP32 với máy tính qua cổng USB và nạp code.

4.  **Vận hành hệ thống:**
    - Cấp nguồn 5V cho toàn bộ hệ thống.
    - Mở **Serial Monitor** trong Arduino IDE (tốc độ 115200) để xem địa chỉ IP của ESP32.
    - Truy cập địa chỉ IP đó từ trình duyệt web để xem giao diện điều khiển.

## 📡 Giao thức Truyền dữ liệu (UART)

#### STM32 → ESP32 (Dữ liệu cảm biến)
-   **Format**: Chuỗi CSV, các giá trị được phân tách bằng dấu phẩy và kết thúc bằng `\n`.
-   **Cấu trúc**: `temperature,humidity,soil_moisture,light,rain\n`
-   **Ví dụ**: `28.5,75.2,65,850,0\n`

#### ESP32 → STM32 (Lệnh điều khiển)
-   **Format**: Ký tự đơn.
-   **Lệnh**:
    -   `'1'` - Bật bơm nước.
    -   `'0'` - Tắt bơm nước.

## 📸 Hình ảnh Thực tế
*Giao diện Web Dashboard*


*Mô hình phần cứng*



## 🧪 Kiểm thử và Hiệu năng
Hệ thống đã được kiểm tra trong môi trường phòng thí nghiệm và cho kết quả tích cực:
-   **Tần suất cập nhật dữ liệu**: 1 lần/giây trên web, 1 lần/15 giây lên ThingSpeak.
-   **Độ chính xác cảm biến**: Sai số dưới ±5% sau khi hiệu chuẩn và lọc nhiễu.
-   **Thời gian phản hồi điều khiển**: Trung bình dưới 200ms từ lúc nhấn nút đến khi relay hoạt động.
-   **Độ ổn định**: Hoạt động liên tục trong 1 giờ mà không có hiện tượng treo máy hay mất kết nối.
-   **Năng lượng tiêu thụ**: Khoảng 130mA @ 5V ở trạng thái chờ.

## 📁 Cấu trúc Thư mục
```
.
├── ESP32_Code                # Mã nguồn cho ESP32
│   └── ESP32_Code.ino
├── STM32_Code                # Project STM32CubeIDE
│   ├── Core
│   ├── Drivers
│   └── ...
├── doc                       # Tài liệu và hình ảnh
│   ├── Báo cáo cuối kỳ.pdf
│   ├── pin-circuit.png
│   └── workflow-diagram.png
└── README.md                 # File README này
```

## 👥 Nhóm phát triển
Dự án được thực hiện bởi nhóm sinh viên Trường Đại học Công nghệ - ĐHQGHN:

| Họ và tên | MSV |
| :--- | :--- |
| **Nguyễn Trọng Nam** | 22022161 |
| **Lê Thế Minh** | 22022215 |
| **Cao Song Toàn** | 22022188 |
| **Doãn Đức Minh** | 22022135 |