#include "button_module.h"

struct ButtonState {
    uint8_t pin;
    bool last_stable;
    bool last_raw;
    unsigned long debounce_time;
    unsigned long press_start;
    bool long_fired;
    ButtonEvent pending;
};

static ButtonState btns[BTN_COUNT];
static const uint8_t pins[BTN_COUNT] = {
    BTN_UP_PIN, BTN_DOWN_PIN, BTN_OK_PIN, BTN_CANCEL_PIN
};

void button_init() {
    for (int i = 0; i < BTN_COUNT; i++) {
        btns[i].pin = pins[i];
        btns[i].last_stable = HIGH;
        btns[i].last_raw = HIGH;
        btns[i].debounce_time = 0;
        btns[i].press_start = 0;
        btns[i].long_fired = false;
        btns[i].pending = EVT_NONE;
        pinMode(btns[i].pin, INPUT_PULLUP);
    }
    Serial.println("[BTN] Init OK (GPIO 0,1,3,4)");
}

void button_update() {
    unsigned long now = millis();

    for (int i = 0; i < BTN_COUNT; i++) {
        bool raw = digitalRead(btns[i].pin);

        // Debounce
        if (raw != btns[i].last_raw) {
            btns[i].debounce_time = now;
            btns[i].last_raw = raw;
        }
        if ((now - btns[i].debounce_time) < DEBOUNCE_MS) continue;

        bool stable = raw;

        // กดลง (HIGH → LOW)
        if (stable == LOW && btns[i].last_stable == HIGH) {
            btns[i].press_start = now;
            btns[i].long_fired = false;
        }

        // กดค้าง → long press
        if (stable == LOW && btns[i].last_stable == LOW && !btns[i].long_fired) {
            if ((now - btns[i].press_start) >= LONG_PRESS_MS) {
                btns[i].pending = EVT_LONG_PRESS;
                btns[i].long_fired = true;
            }
        }

        // ปล่อย (LOW → HIGH)
        if (stable == HIGH && btns[i].last_stable == LOW) {
            if (!btns[i].long_fired) {
                btns[i].pending = EVT_SHORT_PRESS;
            }
        }

        btns[i].last_stable = stable;
    }
}

ButtonEvent button_get_event(ButtonId btn) {
    if (btn >= BTN_COUNT) return EVT_NONE;
    ButtonEvent evt = btns[btn].pending;
    btns[btn].pending = EVT_NONE;
    return evt;
}
