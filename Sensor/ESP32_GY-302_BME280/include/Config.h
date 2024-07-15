#ifndef CONFIG_H
#define CONFIG_H

#include <Preferences.h>
#include "ArduinoJson.h"

namespace
{
    const char *KEY_COLLECT_INTERVAL = "collect_interval";
    const char *KEY_COLLECT_ILLUMINANCE  = "collect_illuminance";

    const char *KEY_PREF_COLLECT_INTERVAL = "clc_intvl";
    const char *KEY_PREF_COLLECT_ILLUMINANCE = "clc_illum";
}

class Config
{
public:
    Config(const char *name = "config");

    const int DEFAULT_COLLECT_INTERVAL = 10;

    void updateConfig(JsonDocument doc);

    int getCollectInterval();

    bool collectIlluminance();

private:
    Preferences preferences;
};

#endif