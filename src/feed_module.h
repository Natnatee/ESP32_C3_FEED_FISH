#pragma once
#include <Arduino.h>

// ===== MOSFET Pin =====
// GPIO10 → OUTPUT, HIGH = จ่ายไฟ 3.3V ตรง
#define FEED_PIN  10

// เริ่มต้น feed module
void feed_init();

// อัพเดท feed timer (เรียกทุก loop)
// interval_h = ทุกกี่ชม., duration_s = จ่ายไฟกี่วิ
void feed_update(uint8_t interval_h, uint8_t duration_s);

// สั่งให้อาหารทันที (manual)
void feed_manual(uint8_t duration_s);

// กำลังให้อาหารอยู่?
bool feed_is_active();

// เวลาที่เหลือก่อนมื้อถัดไป (วินาที)
unsigned long feed_time_remaining(uint8_t interval_h);

// เวลาที่ให้อาหารไปแล้ว (วินาที, ขณะ feeding)
unsigned long feed_elapsed_s();

// เวลาเหลือของ feeding ปัจจุบัน (วินาที)
unsigned long feed_remaining_duration_s();

// จำนวนครั้งที่ให้อาหารแล้วตั้งแต่เปิดเครื่อง
uint32_t feed_get_count();

// countdown ก่อนมื้อถัดไป (format "HH:MM", buf >= 6 bytes)
void feed_get_countdown(uint8_t interval_h, char* buf);

// รีเซ็ท timer (หลังเปลี่ยนค่า interval)
void feed_reset_timer();
