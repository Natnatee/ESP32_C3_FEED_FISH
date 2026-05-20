#include "lcd_module.h"
#include <Wire.h>

static LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

void lcd_init() {
    Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);

    // I2C scan เพื่อ debug
    Serial.println("[LCD] I2C Scanning...");
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("[LCD] Found device at 0x%02X\n", addr);
        }
    }

    lcd.init();
    lcd.backlight();
    lcd.clear();

    // Register custom chars for solid blocky big numbers (0-7)
    uint8_t custom_unused0[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}; // 0: Unused
    uint8_t custom_unused1[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}; // 1: Unused
    uint8_t custom_unused2[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}; // 2: Unused
    uint8_t custom_UB[8]      = {0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00}; // 3: UB (Upper Bar - 3 rows)
    uint8_t custom_LB[8]      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F}; // 4: LB (Lower Bar - 3 rows)
    uint8_t custom_MBT[8]     = {0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x1F, 0x1F}; // 5: MBT (Middle Bar Top-half: 3 top, 2 bottom)
    uint8_t custom_MBB[8]     = {0x1F, 0x1F, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F}; // 6: MBB (Middle Bar Bottom-half: 2 top, 3 bottom)
    uint8_t custom_FB[8]      = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}; // 7: FB (Full Block)

    lcd.createChar(0, custom_unused0);
    lcd.createChar(1, custom_unused1);
    lcd.createChar(2, custom_unused2);
    lcd.createChar(3, custom_UB);
    lcd.createChar(4, custom_LB);
    lcd.createChar(5, custom_MBT);
    lcd.createChar(6, custom_MBB);
    lcd.createChar(7, custom_FB);

    lcd.setCursor(0, 0);
    lcd.print("Fish Feeder");
    lcd.setCursor(0, 1);
    lcd.print("Starting...");
    Serial.println("[LCD] Init OK");
}

void lcd_clear() {
    lcd.clear();
}

void lcd_print(uint8_t col, uint8_t row, const char* text) {
    lcd.setCursor(col, row);
    lcd.print(text);
}

void lcd_show(const char* line1, const char* line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

void lcd_backlight(bool on) {
    if (on) lcd.backlight();
    else lcd.noBacklight();
}

void lcd_create_char(uint8_t location, uint8_t charmap[]) {
    lcd.createChar(location, charmap);
}

LiquidCrystal_I2C& lcd_get() {
    return lcd;
}

void lcd_draw_big_digit(uint8_t val, uint8_t col) {
    if (val > 9) return;
    
    // Custom character table for solid blocky 7-segment digits 0-9 (3 columns wide)
    // No horizontal spacing inside the digits (VL/VR are represented by Full Blocks '7').
    // Vertical spaces inside loops are at least 3 pixels high.
    static const uint8_t big_digits[10][2][3] = {
        {{7, 3, 7}, {7, 4, 7}},       // 0
        {{32, 32, 7}, {32, 32, 7}},   // 1 (Right-aligned to ensure clean digital block continuity)
        {{3, 5, 7}, {7, 6, 4}},       // 2
        {{3, 5, 7}, {4, 6, 7}},       // 3
        {{7, 4, 7}, {32, 3, 7}},      // 4
        {{7, 5, 3}, {4, 6, 7}},       // 5
        {{7, 5, 3}, {7, 6, 7}},       // 6
        {{3, 3, 7}, {32, 32, 7}},     // 7
        {{7, 5, 7}, {7, 6, 7}},       // 8
        {{7, 5, 7}, {4, 6, 7}}        // 9
    };
    
    lcd.setCursor(col, 0);
    lcd.write(big_digits[val][0][0]);
    lcd.write(big_digits[val][0][1]);
    lcd.write(big_digits[val][0][2]);

    lcd.setCursor(col, 1);
    lcd.write(big_digits[val][1][0]);
    lcd.write(big_digits[val][1][1]);
    lcd.write(big_digits[val][1][2]);
}

void lcd_show_big_time(uint8_t hour, uint8_t minute, bool show_colon) {
    // Restored spacious spacing between digits for legibility
    // H1 at col 1, H2 at col 5 (leaves column 4 empty)
    lcd_draw_big_digit(hour / 10, 1);
    lcd_draw_big_digit(hour % 10, 5);
    
    // Draw colon at column 8 (center)
    lcd.setCursor(8, 0);
    lcd.print(show_colon ? "." : " ");
    lcd.setCursor(8, 1);
    lcd.print(show_colon ? "." : " ");
    
    // M1 at col 9, M2 at col 13 (leaves column 12 empty)
    lcd_draw_big_digit(minute / 10, 9);
    lcd_draw_big_digit(minute % 10, 13);
}
