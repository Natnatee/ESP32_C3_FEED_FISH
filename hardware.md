# 📟 MCU Specification: ESP32-C3 Super Mini

### 📋 General Specs
- **Model:** ESP32-C3 (RISC-V Single-Core)
- **Max Clock:** 160 MHz
- **Flash:** 4 MB (Super Mini)
- **SRAM:** 400 KB (Internal)
- **Voltage:** 3.3V (Recommended)
- **Onboard LED:** GPIO 8 (Active Low)

### 🔌 Detailed Pinout & Peripheral Reference
| Peripheral | Pins | Function |
|---|---|---|
| **I2C (LCD)** | **GPIO 21 (SCL), GPIO 20 (SDA)** | LCD1602 I2C (PCF8574) |
| **Button UP** | **GPIO 0** | INPUT_PULLUP, กด = LOW |
| **Button DOWN** | **GPIO 1** | INPUT_PULLUP, กด = LOW |
| **Button OK** | **GPIO 3** | INPUT_PULLUP, กด = LOW |
| **Button CANCEL** | **GPIO 4** | INPUT_PULLUP, กด = LOW |
| **FEED OUT** | **GPIO 10** | OUTPUT, HIGH = จ่ายไฟ 3.3V ตรงให้มอเตอร์ |
| **USB-C** | Internal CDC | Upload / Serial Monitor |

### ⚙️ LCD Display: 1602 I2C
- **Driver:** PCF8574
- **Address:** 0x27 (หรือ 0x3F)
- **Interface:** I2C (Wire)
- **Size:** 16 ตัวอักษร x 2 แถว

### ⚙️ Recommended PlatformIO Config
```ini
[env:esp32c3]
platform = espressif32
board = lolin_c3_mini
framework = arduino
monitor_speed = 115200
board_build.mcu = esp32c3
lib_deps = 
    marcoschwartz/LiquidCrystal_I2C@^1.1.4
```

### ⚠️ Common Hardware Issues / Tips
- **USB CDC on Boot:** มั่นใจว่าเปิดใช้ USB CDC ในบอร์ด `esp32c3` เพื่อให้ดู Log ผ่าน USB ได้
- **Active Low Buttons:** ปุ่ม GPIO 0,1,3,4 ใช้ `INPUT_PULLUP` (กดแล้วเป็น LOW)
- **I2C Address:** LCD1602 มักเป็น 0x27 ถ้าไม่ได้ลอง 0x3F
- **MOSFET:** GPIO 5 สั่ง HIGH = จ่ายไฟ, LOW = หยุด
- **WiFi:** Default SSID: Digitalnatives3, ซิงค์ NTP ทุก 6 ชม., auto reconnect
