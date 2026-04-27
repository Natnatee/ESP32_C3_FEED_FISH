# 🐟 Fish Feeder - ESP32-C3

## สรุปโปรเจกต์
ระบบให้อาหารปลาอัตโนมัติ ใช้ ESP32-C3 Super Mini ควบคุม MOSFET ขับมอเตอร์
แสดงผลผ่าน LCD1602 I2C ตั้งค่าด้วยปุ่ม 4 ปุ่ม ซิงค์เวลาผ่าน WiFi NTP

## Pin Map
| Function | GPIO | Mode |
|---|---|---|
| I2C SDA (LCD) | 20 | I2C |
| I2C SCL (LCD) | 21 | I2C |
| BTN UP | 0 | INPUT_PULLUP |
| BTN DOWN | 1 | INPUT_PULLUP |
| BTN OK | 3 | INPUT_PULLUP |
| BTN CANCEL | 4 | INPUT_PULLUP |
| FEED OUT | 10 | OUTPUT |

## โครงสร้างไฟล์
```
src/
├── main.cpp           # ลูปหลัก
├── config_module.*    # อ่าน/เขียน Preferences
├── button_module.*    # ปุ่มกด 4 ปุ่ม (debounce + long press)
├── lcd_module.*       # จอ LCD1602 I2C
├── wifi_module.*      # WiFi + NTP (sync ทุก 6 ชม.)
├── feed_module.*      # MOSFET timer (auto/manual feed)
└── menu_module.*      # ระบบเมนู (state machine)
```

## ฟีเจอร์
- หน้าจอปกติ: แสดงวันที่/เวลา + countdown ถึงมื้อถัดไป
- เมนู WiFi: แก้ SSID/Password ด้วยปุ่ม 4 ปุ่ม
- เมนู Feed: ตั้ง Interval (1-24h) และ Duration (1-60s)
- Manual Feed: กด OK ค้าง 1 วิ ในหน้าจอปกติ
- Auto Reconnect WiFi ทุก 30 วิ
- NTP Sync ทุก 6 ชม.
- บันทึกค่าลง Flash (Preferences)

## Default Config
- WiFi: Digitalnatives3 / BC202$1$9@
- Feed Interval: 6h
- Feed Duration: 5s
