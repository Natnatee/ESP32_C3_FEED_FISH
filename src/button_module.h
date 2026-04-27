#pragma once
#include <Arduino.h>

// ===== Pin Assignments =====
// BTN1(UP)=GPIO0, BTN2(DOWN)=GPIO1, BTN3(OK)=GPIO3, BTN4(CANCEL)=GPIO4
// INPUT_PULLUP → กด = LOW
#define BTN_UP_PIN     0
#define BTN_DOWN_PIN   1
#define BTN_OK_PIN     2
#define BTN_CANCEL_PIN 3

// ===== Timing =====
#define DEBOUNCE_MS    50
#define LONG_PRESS_MS  1000

// ===== Button ID =====
enum ButtonId {
    BTN_ID_UP = 0,
    BTN_ID_DOWN,
    BTN_ID_OK,
    BTN_ID_CANCEL,
    BTN_COUNT
};

// ===== Button Events =====
enum ButtonEvent {
    EVT_NONE = 0,
    EVT_SHORT_PRESS,
    EVT_LONG_PRESS,
};

void button_init();
void button_update();
ButtonEvent button_get_event(ButtonId btn);
