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

    // feeding → รีเฟรชทุกวิ, normal → รีเฟรชทุกนาที (60000ms)
    unsigned long refresh_ms = feeding_now ? 1000 : 60000;
    if (!need_redraw && (now - last_display_update) < refresh_ms) return;
    last_display_update = now;

    char line1[17], line2[17];
    struct tm ti;
    bool has_time = wifi_get_time(ti);
    const char* days[] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};

    if (has_time) {
        snprintf(line1, 17, "%02d/%02d %s %02d:%02d",
                 ti.tm_mday, ti.tm_mon + 1, days[ti.tm_wday],
                 ti.tm_hour, ti.tm_min);
    } else {
        snprintf(line1, 17, "Setting Time... ");
    }

    if (feeding_now) {
        unsigned long remain = feed_remaining_duration_s();
        snprintf(line2, 17, "FEEDING.. %lus", remain);
    } else {
        char cd[6];
        feed_get_countdown(app_cfg->feed_interval_h, cd);
        snprintf(line2, 17, "-->%s  C:%d", cd, feed_get_count());
    }

    lcd_show(line1, line2);
    need_redraw = false;
}

// ===== แสดงเมนูหลัก =====
static void display_menu_main() {
    if (!need_redraw) return;
    if (menu_cursor == 0)
        lcd_show(">WiFi Settings", " Feed Settings");
    else
        lcd_show(" WiFi Settings", ">Feed Settings");
    need_redraw = false;
}

// ===== แสดงเมนู Feed =====
static void display_feed_menu() {
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

    switch (state) {

    // ==================== NORMAL ====================
    case STATE_NORMAL:
        display_normal();
        if (ok == EVT_SHORT_PRESS) {
            state = STATE_MENU_MAIN;
            menu_cursor = 0;
            need_redraw = true;
        }
        // กด OK ค้าง = Manual Feed
        if (ok == EVT_LONG_PRESS) {
            feed_manual(app_cfg->feed_duration_s);
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
                lcd_show("WiFi Setup Mode", "AP:FishFeederAP");
                wifi_start_portal();
                
                // หลังออกจาก Portal (ไม่ว่าจะต่อสำเร็จ หรือหมดเวลา)
                state = STATE_NORMAL;
            } else {
                state = STATE_FEED_MENU;
            }
            menu_cursor = 0;
            need_redraw = true;
        }
        if (cancel == EVT_SHORT_PRESS) {
            state = STATE_NORMAL;
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
