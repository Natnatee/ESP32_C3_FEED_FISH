#include "wifi_module.h"
#include <WiFi.h>
#include <time.h>
#include <WiFiManager.h>

static bool ntp_synced = false;
static unsigned long last_reconnect_attempt = 0;
static unsigned long last_ntp_sync = 0;

void wifi_init() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(); // ใช้รหัสที่บันทึกไว้ในระบบ

    Serial.println("[WIFI] Auto connecting in background...");

    // ตั้งเวลาเริ่มต้นเป็น 08:00 AM (Unix time: 28800)
    struct tm start_time;
    start_time.tm_year = 2026 - 1900;
    start_time.tm_mon = 0;
    start_time.tm_mday = 1;
    start_time.tm_hour = 1; // 01:00 UTC + 7 = 08:00 Thai
    start_time.tm_min = 0;
    start_time.tm_sec = 0;
    time_t t = mktime(&start_time);
    struct timeval tv = { .tv_sec = t };
    settimeofday(&tv, NULL);

    // ไม่รอต่อ WiFi แบบ blocking ให้ไปต่อในลูปเอง
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    last_ntp_sync = millis();
}

void wifi_update() {
    unsigned long now = millis();

    // ถ้าหลุด → พยายาม reconnect ทุก 30 วิ
    if (WiFi.status() != WL_CONNECTED) {
        ntp_synced = false;
        if ((now - last_reconnect_attempt) >= WIFI_RECONNECT_INTERVAL) {
            last_reconnect_attempt = now;
            Serial.println("[WIFI] Reconnecting...");
            WiFi.disconnect();
            WiFi.begin();
        }
        return;
    }

    // ซิงค์ NTP ทุก 6 ชม.
    if ((now - last_ntp_sync) >= NTP_SYNC_INTERVAL_MS || !ntp_synced) {
        configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
        last_ntp_sync = now;
        ntp_synced = true;
        Serial.println("[WIFI] NTP re-synced");
    }
}

void wifi_start_portal() {
    WiFiManager wm;
    wm.setConfigPortalTimeout(180); // 3 นาที timeout
    
    Serial.println("[WIFI] Starting AP Mode: FishFeederAP");
    if (!wm.startConfigPortal("FishFeederAP")) {
        Serial.println("[WIFI] Config timeout or exited");
    } else {
        Serial.println("[WIFI] Connected to new WiFi!");
        // เร่งการซิงค์เวลา
        configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
        last_ntp_sync = millis();
        ntp_synced = true;
    }
}

bool wifi_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}

bool wifi_ntp_synced() {
    return ntp_synced;
}

bool wifi_get_time(struct tm &timeinfo) {
    return getLocalTime(&timeinfo, 100);
}
