#include "menu_module.h"
#include "button_module.h"
#include "lcd_module.h"
#include "feed_module.h"
#include "wifi_module.h"
#include <time.h>

// ===== State =====
static AppState state = STATE_NORMAL;
static AppConfig* app_cfg = nullptr;
static int menu_cursor = 0;          // ตำแหน่ง cursor ในเมนู
static bool need_redraw = true;

// ===== Normal Mode Pages =====
static int normal_page = 0;          // 0: Time, 1: Next Feed, 2: WiFi SSID, 3: Dark Screen
static unsigned long last_normal_interaction = 0;
#define NORMAL_PAGE_TIMEOUT_MS 900000UL // 15 นาที (15 * 60 * 1000)

// ===== Number editor =====
static uint8_t edit_num = 0;
static uint8_t edit_num_min = 1;
static uint8_t edit_num_max = 24;

// ===== Display helpers =====
static unsigned long last_display_update = 0;
static bool last_feeding_state = false;
#define DISPLAY_UPDATE_MS 500

// ===== แสดงหน้าจอปกติ =====
static void display_normal() {
    unsigned long now = millis();
    bool feeding_now = feed_is_active();

    // ตรวจจับเมื่อ feeding state เปลี่ยน → บังคับ redraw
    if (feeding_now != last_feeding_state) {
        last_feeding_state = feeding_now;
        need_redraw = true;
    }

    // กำหนดความเร็วในการรีเฟรช:
    // - โหมดให้อาหาร: รีเฟรชทุก 1 วินาที
    // - หน้าเวลา (Page 0): รีเฟรชทุก 500ms (เพื่อกระพริบจุดคั่นเวลา)
    // - หน้าอื่น: รีเฟรชทุก 5 วินาที
    unsigned long refresh_ms = 5000;
    if (feeding_now) {
        refresh_ms = 1000;
    } else if (normal_page == 0) {
        refresh_ms = 500;
    }

    if (!need_redraw && (now - last_display_update) < refresh_ms) return;
    last_display_update = now;

    // ถ้ากำลังให้อาหารปลาอยู่ ให้แสดงหน้าจอ Feeding เต็มจอตลอดไม่ว่าจะอยู่หน้าไหน
    if (feeding_now) {
        lcd_backlight(true);
        if (need_redraw) {
            lcd_clear();
        }
        unsigned long remain = feed_remaining_duration_s();
        char line1[17], line2[17];
        snprintf(line1, 17, "  FEEDING NOW   ");
        snprintf(line2, 17, "  REMAIN: %2lus   ");
        snprintf(line2, 17, "  REMAIN: %2lus   ", remain);
        lcd_print(0, 0, line1);
        lcd_print(0, 1, line2);
        need_redraw = false;
        return;
    }

    if (normal_page == 3) {
        // --- หน้าที่ 4: หน้าจอมืด (ปิดไฟ Backlight) ---
        lcd_backlight(false);
        if (need_redraw) {
            lcd_clear();
        }
    } 
    else {
        // หน้า 0, 1, 2 เปิดไฟ Backlight
        lcd_backlight(true);

        // ล้างจอก่อนวาดหน้าใหม่หากมีการเปลี่ยนหน้า
        if (need_redraw) {
            lcd_clear();
        }

        struct tm ti;
        bool has_time = wifi_get_time(ti);

        if (normal_page == 0) {
            // --- หน้าที่ 1: หน้าจอเวลาตัวใหญ่ ---
            if (has_time) {
                // กระพริบจุดคั่นเวลาทุกๆ 500ms
                bool show_colon = (millis() / 500) % 2;
                lcd_show_big_time(ti.tm_hour, ti.tm_min, show_colon);
            } else {
                lcd_print(0, 0, "  Setting Time  ");
                lcd_print(0, 1, "  Please wait   ");
            }
        } 
        else if (normal_page == 1) {
            // --- หน้าที่ 2: รอบการให้อาหารครั้งถัดไป ---
            char line1[17], line2[17];
            char cd[6];
            feed_get_countdown(app_cfg->feed_interval_h, cd);
            snprintf(line1, 17, " NEXT FEED TIME ");
            snprintf(line2, 17, "  -->%s   C:%-3d ", cd, feed_get_count());
            lcd_print(0, 0, line1);
            lcd_print(0, 1, line2);
        } 
        else if (normal_page == 2) {
            // --- หน้าที่ 3: สถานะ Wi-Fi ---
            char line1[17], line2[17];
            snprintf(line1, 17, "  WiFi Network  ");
            if (wifi_is_connected()) {
                String ssid = wifi_get_ssid();
                int len = ssid.length();
                if (len > 16) {
                    ssid = ssid.substring(0, 16);
                    len = 16;
                }
                int spaces = (16 - len) / 2;
                char temp[17];
                memset(temp, ' ', 16);
                temp[16] = '\0';
                memcpy(temp + spaces, ssid.c_str(), len);
                snprintf(line2, 17, "%s", temp);
            } else {
                snprintf(line2, 17, "    No wifi     ");
            }
            lcd_print(0, 0, line1);
            lcd_print(0, 1, line2);
        }
    }

    need_redraw = false;
}

// ===== แสดงเมนูหลัก =====
static void display_menu_main() {
    lcd_backlight(true);
    if (!need_redraw) return;
    if (menu_cursor == 0)
        lcd_show(">WiFi Settings", " Feed Settings");
    else
        lcd_show(" WiFi Settings", ">Feed Settings");
    need_redraw = false;
}

// ===== แสดงเมนู Feed =====
static void display_feed_menu() {
    lcd_backlight(true);
    if (!need_redraw) return;
    char l1[17], l2[17];
    if (menu_cursor == 0) {
        snprintf(l1, 17, ">Interval: %dh", app_cfg->feed_interval_h);
        snprintf(l2, 17, " Duration: %ds", app_cfg->feed_duration_s);
    } else {
        snprintf(l1, 17, " Interval: %dh", app_cfg->feed_interval_h);
        snprintf(l2, 17, ">Duration: %ds", app_cfg->feed_duration_s);
    }
    lcd_show(l1, l2);
    need_redraw = false;
}

// ===== แสดง Number editor =====
static void display_number_editor() {
    lcd_backlight(true);
    if (!need_redraw) return;
    char l1[17], l2[17];

    if (state == STATE_FEED_EDIT_INTV) {
        snprintf(l1, 17, "Feed Interval:");
        snprintf(l2, 17, "< %d > hours", edit_num);
    } else {
        snprintf(l1, 17, "Feed Duration:");
        snprintf(l2, 17, "< %d > seconds", edit_num);
    }
    lcd_show(l1, l2);
    need_redraw = false;
}

// ===== Menu Init =====
void menu_init(AppConfig* cfg) {
    app_cfg = cfg;
    state = STATE_NORMAL;
    
    // ตรวจสอบเวลาช่วงเริ่มต้น
    struct tm ti;
    bool has_time = wifi_get_time(ti);
    if (has_time && (ti.tm_hour >= 22 || ti.tm_hour < 5)) {
        normal_page = 3;
    } else {
        normal_page = 0;
    }
    
    last_normal_interaction = millis();
    need_redraw = true;
    Serial.println("[MENU] Init OK");
}

// ===== Menu Update (return true if config changed) =====
bool menu_update() {
    ButtonEvent up   = button_get_event(BTN_ID_UP);
    ButtonEvent down = button_get_event(BTN_ID_DOWN);
    ButtonEvent ok   = button_get_event(BTN_ID_OK);
    ButtonEvent cancel = button_get_event(BTN_ID_CANCEL);
    bool config_changed = false;
    unsigned long now = millis();

    // 1. ตรวจจับปุ่มกดใดๆ เพื่อรีเซ็ตเวลา Inactivity
    bool has_interaction = (up != EVT_NONE || down != EVT_NONE || ok != EVT_NONE || cancel != EVT_NONE);
    if (has_interaction) {
        last_normal_interaction = now;
    }

    // 2. ตรวจสอบช่วงเวลา (หลังสี่ทุ่ม 22:00 ถึง ตีห้า 05:00)
    struct tm ti;
    bool has_time = wifi_get_time(ti);
    bool is_nighttime = false;
    if (has_time) {
        is_nighttime = (ti.tm_hour >= 22 || ti.tm_hour < 5);
    }
    int target_default_page = is_nighttime ? 3 : 0;

    // 3. ตรวจสอบ Inactivity Timeout 15 นาที
    if ((now - last_normal_interaction) >= NORMAL_PAGE_TIMEOUT_MS) {
        if (state != STATE_NORMAL || normal_page != target_default_page) {
            state = STATE_NORMAL;
            normal_page = target_default_page;
            need_redraw = true;
        }
    }

    switch (state) {

    // ==================== NORMAL ====================
    case STATE_NORMAL:
        display_normal();

        // กด UP/DOWN เลื่อนเปลี่ยนหน้าจอวนลูป 4 หน้า
        if (up == EVT_SHORT_PRESS) {
            normal_page = (normal_page - 1 + 4) % 4;
            last_normal_interaction = now;
            need_redraw = true;
        }
        if (down == EVT_SHORT_PRESS) {
            normal_page = (normal_page + 1) % 4;
            last_normal_interaction = now;
            need_redraw = true;
        }

        // กด Cancel = เข้าสู่เมนูหลัก
        if (cancel == EVT_SHORT_PRESS) {
            state = STATE_MENU_MAIN;
            menu_cursor = 0;
            need_redraw = true;
        }
        // กด OK ธรรมดา = Manual Feed (สั่งให้อาหารปลาทันที)
        if (ok == EVT_SHORT_PRESS) {
            feed_manual(app_cfg->feed_duration_s);
            last_normal_interaction = now;
            need_redraw = true;
        }
        break;

    // ==================== MAIN MENU ====================
    case STATE_MENU_MAIN:
        display_menu_main();
        if (up == EVT_SHORT_PRESS || down == EVT_SHORT_PRESS) {
            menu_cursor = 1 - menu_cursor;  // toggle 0/1
            need_redraw = true;
        }
        if (ok == EVT_SHORT_PRESS) {
            if (menu_cursor == 0) {
                // เข้าสู่ WiFi Portal โหมด
                lcd_backlight(true);
                lcd_show("WiFi Setup Mode", "AP:FishFeederAP");
                wifi_start_portal();
                
                // หลังออกจาก Portal (ไม่ว่าจะต่อสำเร็จ หรือหมดเวลา)
                state = STATE_NORMAL;
                struct tm ti_exit;
                if (wifi_get_time(ti_exit) && (ti_exit.tm_hour >= 22 || ti_exit.tm_hour < 5)) {
                    normal_page = 3;
                } else {
                    normal_page = 0;
                }
                last_normal_interaction = millis();
            } else {
                state = STATE_FEED_MENU;
            }
            menu_cursor = 0;
            need_redraw = true;
        }
        if (cancel == EVT_SHORT_PRESS) {
            state = STATE_NORMAL;
            normal_page = target_default_page;
            last_normal_interaction = now;
            need_redraw = true;
        }
        break;

    // ==================== FEED MENU ====================
    case STATE_FEED_MENU:
        display_feed_menu();
        if (up == EVT_SHORT_PRESS || down == EVT_SHORT_PRESS) {
            menu_cursor = 1 - menu_cursor;
            need_redraw = true;
        }
        if (ok == EVT_SHORT_PRESS) {
            if (menu_cursor == 0) {
                state = STATE_FEED_EDIT_INTV;
                edit_num = app_cfg->feed_interval_h;
                edit_num_min = 1;
                edit_num_max = 24;
            } else {
                state = STATE_FEED_EDIT_DUR;
                edit_num = app_cfg->feed_duration_s;
                edit_num_min = 1;
                edit_num_max = 60;
            }
            need_redraw = true;
        }
        if (cancel == EVT_SHORT_PRESS) {
            state = STATE_MENU_MAIN;
            menu_cursor = 1;
            need_redraw = true;
        }
        break;

    // ==================== FEED EDIT (Number) ====================
    case STATE_FEED_EDIT_INTV:
    case STATE_FEED_EDIT_DUR:
        display_number_editor();
        if (up == EVT_SHORT_PRESS) {
            if (edit_num < edit_num_max) edit_num++;
            else edit_num = edit_num_min;
            need_redraw = true;
        }
        if (down == EVT_SHORT_PRESS) {
            if (edit_num > edit_num_min) edit_num--;
            else edit_num = edit_num_max;
            need_redraw = true;
        }
        if (ok == EVT_SHORT_PRESS) {
            if (state == STATE_FEED_EDIT_INTV) {
                app_cfg->feed_interval_h = edit_num;
                feed_reset_timer();
            } else {
                app_cfg->feed_duration_s = edit_num;
            }
            config_changed = true;
            state = STATE_FEED_MENU;
            menu_cursor = 0;
            need_redraw = true;
            lcd_show("Saved!", "");
            delay(500);
            need_redraw = true;
            Serial.printf("[MENU] Feed cfg: %dh / %ds\n",
                          app_cfg->feed_interval_h, app_cfg->feed_duration_s);
        }
        if (cancel == EVT_SHORT_PRESS) {
            state = STATE_FEED_MENU;
            need_redraw = true;
        }
        break;
    }

    return config_changed;
}

AppState menu_get_state() {
    return state;
}
