#pragma once
#include <Arduino.h>
#include "config_module.h"

// ===== App States =====
enum AppState {
    STATE_NORMAL,           // แสดงวันที่/เวลา
    STATE_MENU_MAIN,        // เมนูหลัก (WiFi / Feed)
    STATE_FEED_MENU,        // เมนู Feed (Interval / Duration)
    STATE_FEED_EDIT_INTV,   // แก้ Interval
    STATE_FEED_EDIT_DUR,    // แก้ Duration
};

// เริ่มต้น menu
void menu_init(AppConfig* cfg);

// อัพเดท menu (เรียกทุก loop) - return true ถ้า config เปลี่ยน
bool menu_update();

// ดึง state ปัจจุบัน
AppState menu_get_state();
