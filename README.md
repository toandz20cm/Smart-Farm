# Smart Agriculture Monitoring System 🌱

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![STM32](https://img.shields.io/badge/STM32-Microcontroller-blue.svg)](https://www.st.com/en/microcontrollers-microprocessors/stm32-32-bit-arm-cortex-mcus.html)
[![ESP32](https://img.shields.io/badge/ESP32-WiFi%20Module-red.svg)](https://www.espressif.com/en/products/socs/esp32)

> **Hệ thống giám sát nông nghiệp thông minh sử dụng IoT để tối ưu hóa quá trình canh tác**

[Video thực hành](https://youtu.be/CVzV6Cb0qsQ?feature=shared)


## 📋 Tổng quan dự án

Hệ thống Smart Agriculture Monitoring được thiết kế để giải quyết các thách thức trong nông nghiệp Việt Nam thông qua việc ứng dụng công nghệ IoT và tự động hóa. Với hơn 60% dân số phụ thuộc vào sản xuất nông nghiệp, hệ thống này giúp:

- 📊 **Thu thập dữ liệu**: Giám sát liên tục các thông số môi trường quan trọng
- 🌐 **Giám sát từ xa**: Hiển thị dữ liệu thời gian thực qua giao diện web
- 💧 **Điều khiển tự động**: Bật/tắt hệ thống tưới tiêu dựa trên ngưỡng cài đặt

## 🛠️ Công nghệ sử dụng

### Phần cứng
- **Vi điều khiển**: STM32 (ARM Cortex-M, 32-bit)
- **Module WiFi**: ESP32 (Dual-core, 240MHz)
- **Cảm biến**: 
  - Cảm biến độ ẩm đất (Soil Moisture Sensor)
  - DHT11 (Nhiệt độ/Độ ẩm không khí)
  - Cảm biến ánh sáng (Photodiode)
  - Cảm biến mưa (Rain Sensor)
- **Thiết bị điều khiển**: Relay + Máy bơm nước

### Phần mềm
- **STM32**: C/C++ với STM32CubeIDE
- **ESP32**: Arduino IDE với C++
- **Giao tiếp**: UART (9600 bps)
- **Web Interface**: HTML/CSS/JavaScript
- **Xử lý tín hiệu**: Moving Average Filter, Median Filter

## 🏗️ Kiến trúc hệ thống

```
┌─────────────────┐    UART     ┌─────────────────┐    WiFi    ┌─────────────────┐
│                 │ ──────────► │                 │ ─────────► │                 │
│     STM32       │             │     ESP32       │            │   Web Browser   │
│  (Data Process) │ ◄────────── │  (Web Server)   │ ◄───────── │   (Dashboard)   │
│                 │             │                 │            │                 │
└─────────────────┘             └─────────────────┘            └─────────────────┘
         │                               │
         ▼                               ▼
┌─────────────────┐             ┌─────────────────┐
│    Sensors      │             │   Pump Control  │
│ • Soil Moisture │             │     (Relay)     │
│ • DHT11         │             │                 │
│ • Light Sensor  │             │                 │
│ • Rain Sensor   │             │                 │
└─────────────────┘             └─────────────────┘
```

## 📈 Tính năng chính

### 🔍 Giám sát môi trường
- **Độ ẩm đất**: Đo độ ẩm qua cảm biến điện dung
- **Nhiệt độ/Độ ẩm**: Theo dõi điều kiện khí hậu qua DHT11
- **Ánh sáng**: Đánh giá điều kiện quang hợp
- **Lượng mưa**: Phát hiện và đo lường lượng mưa

### 🎛️ Điều khiển từ xa
- **Giao diện web**: Dashboard hiển thị dữ liệu real-time
- **Điều khiển bơm**: Bật/tắt máy bơm từ xa
- **Trạng thái hệ thống**: Theo dõi tình trạng thiết bị

### 🔧 Xử lý dữ liệu
- **Lọc nhiễu**: Moving Average và Median Filter
- **Định dạng dữ liệu**: CSV format cho việc truyền tải
- **Cập nhật**: Chu kỳ 2 giây cho mỗi lần đọc cảm biến

## 🚀 Cài đặt và sử dụng

### Yêu cầu hệ thống
- STM32CubeIDE
- Arduino IDE
- ST-Link Debugger
- WiFi Router

### Cài đặt phần cứng

#### STM32 Pin Configuration
```
PA0 - Cảm biến độ ẩm đất (ADC)
PA1 - DHT11 (Digital I/O)
PA2 - UART TX (ESP32)
PA3 - UART RX (ESP32)
PA4 - Cảm biến ánh sáng (ADC)
PA5 - Cảm biến mưa (ADC)
PA6 - Relay điều khiển bơm
```

#### ESP32 Pin Configuration
```
GPIO pins theo cấu hình UART
RX - Nhận dữ liệu từ STM32
TX - Gửi lệnh điều khiển về STM32
```

### Cài đặt phần mềm

1. **Clone repository**

2. **Nạp code cho STM32**


3. **Nạp code cho ESP32**


4. **Cấu hình WiFi**

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

## 📊 Giao thức truyền dữ liệu

### STM32 → ESP32 (Sensor Data)
```
Format: moisture,temperature,humidity,light,rain\n
Example: 45,27,75,300,10\n
```

### ESP32 → STM32 (Control Commands)
```
'1' - Bật bơm nước
'0' - Tắt bơm nước  
'?' - Truy vấn trạng thái bơm
```

## 🌐 Giao diện Web

Truy cập dashboard qua địa chỉ IP của ESP32:
```
http://192.168.1.XXX
```

### Endpoints API
- `/` - Dashboard chính
- `/on` - Bật bơm nước
- `/off` - Tắt bơm nước
- `/status` - Kiểm tra trạng thái

## 📸 Screenshots

*Thêm screenshots của giao diện web và setup phần cứng*

## 🧪 Kiểm thử

### Test Cases
.......

### Performance
- **Tần suất cập nhật**: 0.5 giây/lần
- **Độ chính xác**: ±2% cho các cảm biến analog
- **Thời gian phản hồi**: <1 giây cho lệnh điều khiển

## 📁 Cấu trúc dự án

....

## 👥 Nhóm phát triển

- **Nguyễn Trọng Nam** - MSV: 22022161
- **Doãn Đức Minh** - MSV: 22022135  
- **Lê Thế Minh** - MSV: 22022215
- **Cao Song Toàn** - MSV: 22022188

*Trường Đại học Công nghệ - Đại học Quốc gia Hà Nội*