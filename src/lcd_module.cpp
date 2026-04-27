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
