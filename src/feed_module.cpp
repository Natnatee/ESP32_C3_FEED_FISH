#include "feed_module.h"

static bool is_feeding = false;
static unsigned long feed_start_time = 0;
static unsigned long feed_duration_ms = 0;
static unsigned long last_feed_time = 0;
static uint32_t feed_count = 0;

void feed_init() {
    pinMode(FEED_PIN, OUTPUT);
    digitalWrite(FEED_PIN, LOW);  // ปิดมอเตอร์
    last_feed_time = millis();
    Serial.println("[FEED] Init OK (GPIO10)");
}

void feed_update(uint8_t interval_h, uint8_t duration_s) {
    unsigned long now = millis();
    unsigned long interval_ms = (unsigned long)interval_h * 3600000UL;

    // ถึงเวลาให้อาหาร?
    if (!is_feeding && (now - last_feed_time) >= interval_ms) {
        Serial.println("[FEED] Auto feed triggered!");
        feed_manual(duration_s);
    }

    // กำลังให้อาหาร → เช็คว่าครบเวลาหรือยัง
    if (is_feeding) {
        if ((now - feed_start_time) >= feed_duration_ms) {
            digitalWrite(FEED_PIN, LOW);
            is_feeding = false;
            Serial.println("[FEED] Feed complete");
        }
    }
}

void feed_manual(uint8_t duration_s) {
    if (is_feeding) return;  // กำลังให้อยู่แล้ว
    is_feeding = true;
    feed_count++;  // เพิ่มจำนวนครั้ง
    feed_start_time = millis();
    feed_duration_ms = (unsigned long)duration_s * 1000UL;
    last_feed_time = millis();
    digitalWrite(FEED_PIN, HIGH);
    Serial.printf("[FEED] Feeding for %d seconds (Count: %d)\n", duration_s, feed_count);
}

bool feed_is_active() {
    return is_feeding;
}

unsigned long feed_time_remaining(uint8_t interval_h) {
    unsigned long now = millis();
    unsigned long interval_ms = (unsigned long)interval_h * 3600000UL;
    unsigned long elapsed = now - last_feed_time;
    if (elapsed >= interval_ms) return 0;
    return (interval_ms - elapsed) / 1000;  // คืนค่าเป็นวินาที
}

unsigned long feed_elapsed_s() {
    if (!is_feeding) return 0;
    return (millis() - feed_start_time) / 1000;
}

unsigned long feed_remaining_duration_s() {
    if (!is_feeding) return 0;
    unsigned long elapsed_ms = millis() - feed_start_time;
    if (elapsed_ms >= feed_duration_ms) return 0;
    return (feed_duration_ms - elapsed_ms) / 1000;
}

uint32_t feed_get_count() {
    return feed_count;
}

void feed_get_countdown(uint8_t interval_h, char* buf) {
    unsigned long remain_s = feed_time_remaining(interval_h);
    unsigned long h = remain_s / 3600;
    unsigned long m = (remain_s % 3600) / 60;
    snprintf(buf, 6, "%02lu:%02lu", h, m);
}

void feed_reset_timer() {
    last_feed_time = millis();
    Serial.println("[FEED] Timer reset");
}
