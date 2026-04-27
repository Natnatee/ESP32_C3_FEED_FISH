// =============================================
// Fish Feeder Controller - ESP32-C3 Super Mini
// LCD1602 I2C + 4 Buttons + MOSFET Motor
// WiFi NTP Auto Sync + Auto Feed Timer
// =============================================

#include <Arduino.h>
#include "config_module.h"
#include "button_module.h"
#include "lcd_module.h"
#include "wifi_module.h"
#include "feed_module.h"
#include "menu_module.h"

static AppConfig cfg;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Fish Feeder v1.0 ===");

    // 1. โหลด config จาก flash
    config_init();
    config_load(cfg);

    // 2. เริ่มต้น hardware
    button_init();
    lcd_init();
    feed_init();

    // 3. เชื่อมต่อ WiFi + NTP
    wifi_init();

    // 4. เริ่มต้น menu
    menu_init(&cfg);

    Serial.println("=== Setup Complete ===\n");
}

void loop() {
    // 1. อัพเดทปุ่มกด
    button_update();

    // 2. อัพเดท menu + ตรวจสอบว่า config เปลี่ยนหรือไม่
    bool cfg_changed = menu_update();

    // 3. ถ้า config เปลี่ยน → บันทึกลง flash
    if (cfg_changed) {
        config_save(cfg);
    }

    // 4. อัพเดท WiFi (reconnect + NTP sync)
    wifi_update();

    // 5. อัพเดท feed timer (auto feed)
    feed_update(cfg.feed_interval_h, cfg.feed_duration_s);
}
