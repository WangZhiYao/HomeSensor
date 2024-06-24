#ifndef CONFIG_H
#define CONFIG_H

#include <Preferences.h>

namespace
{
    const char *KEY_PUBLISH_INTERVAL = "pub_interval";
}

class Config
{
public:
    Config(const char *name);

    const int DEFAULT_PUBLISH_INTERVAL = 10;

    int getPublishInterval();

    bool setPublishInterval(int publishInterval);

private:
    Preferences preferences;
};

#endif