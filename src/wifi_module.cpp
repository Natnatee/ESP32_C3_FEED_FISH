#include "wifi_module.h"
#include <WiFi.h>
#include <time.h>
#include <WiFiManager.h>

enum WifiState {
    WIFI_STATE_TRY_SAVED,
    WIFI_STATE_TRY_FALLBACK,
    WIFI_STATE_COOLDOWN,
    WIFI_STATE_CONNECTED
};

static WifiState current_wifi_state = WIFI_STATE_TRY_SAVED;
static unsigned long state_start_time = 0;
static bool ntp_synced = false;
static unsigned long last_ntp_sync = 0;

void wifi_init() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(); // ใช้รหัสที่บันทึกไว้ในระบบ
    current_wifi_state = WIFI_STATE_TRY_SAVED;
    state_start_time = millis();

    Serial.println("[WIFI] Auto connecting to saved network in background...");

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

    if (WiFi.status() == WL_CONNECTED) {
        if (current_wifi_state != WIFI_STATE_CONNECTED) {
            current_wifi_state = WIFI_STATE_CONNECTED;
            Serial.println("[WIFI] Connected!");
            configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
            last_ntp_sync = now;
            ntp_synced = true;
        }

        // ซิงค์ NTP ทุก 6 ชม.
        if ((now - last_ntp_sync) >= NTP_SYNC_INTERVAL_MS || !ntp_synced) {
            configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
            last_ntp_sync = now;
            ntp_synced = true;
            Serial.println("[WIFI] NTP re-synced");
        }
        return;
    }

    // ถ้าหลุด
    ntp_synced = false;

    switch (current_wifi_state) {
        case WIFI_STATE_CONNECTED:
            // เพิ่งหลุด
            Serial.println("[WIFI] Disconnected. Trying saved network...");
            WiFi.disconnect();
            WiFi.begin();
            current_wifi_state = WIFI_STATE_TRY_SAVED;
            state_start_time = now;
            break;

        case WIFI_STATE_TRY_SAVED:
            if (now - state_start_time >= WIFI_CONNECT_TIMEOUT_MS) { // 10 วินาที
                Serial.println("[WIFI] Saved network failed. Trying fallback...");
                WiFi.disconnect();
                WiFi.begin("omgdigital_4064", "60F&003ii4@4");
                current_wifi_state = WIFI_STATE_TRY_FALLBACK;
                state_start_time = now;
            }
            break;

        case WIFI_STATE_TRY_FALLBACK:
            if (now - state_start_time >= WIFI_CONNECT_TIMEOUT_MS) { // 10 วินาที
                Serial.println("[WIFI] Fallback network failed. Entering 10 min cooldown...");
                WiFi.disconnect();
                current_wifi_state = WIFI_STATE_COOLDOWN;
                state_start_time = now;
            }
            break;

        case WIFI_STATE_COOLDOWN:
            if (now - state_start_time >= 600000) { // 10 นาที (600,000 ms)
                Serial.println("[WIFI] Cooldown finished. Trying saved network again...");
                WiFi.begin();
                current_wifi_state = WIFI_STATE_TRY_SAVED;
                state_start_time = now;
            }
            break;
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

String wifi_get_ssid() {
    return WiFi.SSID();
}
