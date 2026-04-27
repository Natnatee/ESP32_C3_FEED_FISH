#include "config_module.h"
#include <Preferences.h>

static Preferences prefs;

void config_init() {
    prefs.begin("fishfeed", false);
    Serial.println("[CONFIG] Preferences initialized");
}

void config_load(AppConfig &cfg) {
    cfg.feed_interval_h = prefs.getUChar("interval", DEFAULT_FEED_INTERVAL_H);
    cfg.feed_duration_s = prefs.getUChar("duration", DEFAULT_FEED_DURATION_S);

    // Clamp values
    cfg.feed_interval_h = constrain(cfg.feed_interval_h, 1, 24);
    cfg.feed_duration_s = constrain(cfg.feed_duration_s, 1, 60);

    Serial.printf("[CONFIG] Loaded - Intv:%dh Dur:%ds\n",
                  cfg.feed_interval_h, cfg.feed_duration_s);
}

void config_save(const AppConfig &cfg) {
    prefs.putUChar("interval", cfg.feed_interval_h);
    prefs.putUChar("duration", cfg.feed_duration_s);

    Serial.printf("[CONFIG] Saved - Intv:%dh Dur:%ds\n",
                  cfg.feed_interval_h, cfg.feed_duration_s);
}
