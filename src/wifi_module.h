#pragma once
#include <Arduino.h>

// ===== WiFi Config =====
#define WIFI_CONNECT_TIMEOUT_MS  10000   // timeout ต่อ WiFi 10 วิ
#define WIFI_RECONNECT_INTERVAL  30000   // ลอง reconnect ทุก 30 วิ
#define NTP_SYNC_INTERVAL_MS     21600000UL  // ซิงค์ NTP ทุก 6 ชม.
#define NTP_SERVER               "pool.ntp.org"
#define GMT_OFFSET_SEC           25200    // GMT+7 = 7*3600
#define DAYLIGHT_OFFSET_SEC      0

// เริ่มต้น WiFi + NTP
void wifi_init();

// อัพเดท WiFi state (reconnect, NTP sync) - เรียกทุก loop
void wifi_update();

// เปิดหน้าตั้งค่า WiFi (AP Mode)
void wifi_start_portal();

// สถานะ
bool wifi_is_connected();
bool wifi_ntp_synced();
String wifi_get_ssid();

// ดึงเวลาปัจจุบัน (struct tm)
bool wifi_get_time(struct tm &timeinfo);
