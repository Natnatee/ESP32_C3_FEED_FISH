#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// ===== LCD Pin Config =====
// SDA → GPIO20, SCL → GPIO21
#define LCD_SDA_PIN  20
#define LCD_SCL_PIN  21
#define LCD_ADDR     0x27  // ลอง 0x3F ถ้าจอไม่ติด
#define LCD_COLS     16
#define LCD_ROWS     2

// เริ่มต้น LCD (เรียก Wire.begin ด้วย)
void lcd_init();

// เคลียร์จอ
void lcd_clear();

// แสดงข้อความที่ตำแหน่ง (col, row)
void lcd_print(uint8_t col, uint8_t row, const char* text);

// แสดงข้อความเต็มทั้ง 2 บรรทัด
void lcd_show(const char* line1, const char* line2);

// แสดงเวลาตัวใหญ่
void lcd_show_big_time(uint8_t hour, uint8_t minute, bool show_colon);
void lcd_draw_big_digit(uint8_t val, uint8_t col);

// เปิด/ปิด backlight
void lcd_backlight(bool on);

// สร้าง custom character
void lcd_create_char(uint8_t location, uint8_t charmap[]);

// ดึง reference ของ LCD object
LiquidCrystal_I2C& lcd_get();
