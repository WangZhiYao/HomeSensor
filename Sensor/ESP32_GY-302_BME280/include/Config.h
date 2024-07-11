#ifndef CONFIG_H
#define CONFIG_H

#include <Preferences.h>
#include "ArduinoJson.h"

namespace
{
    const char *KEY_PUBLISH_INTERVAL = "pub_interval";
    const char *KEY_LIGHT_ENABLE = "light_enable";
}

class Config
{
public:
    Config(const char *name = "config");

    const int DEFAULT_PUBLISH_INTERVAL = 10;

    void updateConfig(JsonDocument doc);

    int getPublishInterval();

    bool isLightEnable();

private:
    Preferences preferences;
};

#endif