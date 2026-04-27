#pragma once
#include <Arduino.h>

// ===== Default Config =====
#define DEFAULT_FEED_INTERVAL_H 24   // ทุก 24 ชม.
#define DEFAULT_FEED_DURATION_S 5    // จ่ายไฟ 5 วิ

struct AppConfig {
    uint8_t feed_interval_h;  // 1-24 ชม.
    uint8_t feed_duration_s;  // 1-60 วิ
};

// เริ่มต้น Preferences
void config_init();

// โหลดค่าจาก flash (ถ้าไม่มีจะใช้ค่า default)
void config_load(AppConfig &cfg);

// บันทึกค่าลง flash
void config_save(const AppConfig &cfg);
