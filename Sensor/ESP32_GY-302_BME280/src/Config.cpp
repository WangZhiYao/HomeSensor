#include "Config.h"
#include "nvs_flash.h"

Config::Config(const char *name)
{
    // Initialize the NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    preferences.begin(name);
}

void Config::updateConfig(JsonDocument doc)
{
    if (doc.containsKey(KEY_PUBLISH_INTERVAL))
    {
        int publishInterval = doc[KEY_PUBLISH_INTERVAL].as<int>();
        Serial.print("Set publish interval to: ");
        Serial.println(publishInterval);
        preferences.putInt(KEY_PUBLISH_INTERVAL, publishInterval);
    }
    if (doc.containsKey(KEY_LIGHT_ENABLE))
    {
        bool lightEnable = doc[KEY_LIGHT_ENABLE].as<bool>();
        Serial.print("Set light enable to: ");
        Serial.println(lightEnable);
        preferences.putBool(KEY_LIGHT_ENABLE, lightEnable);
    }
}

int Config::getPublishInterval()
{
    return preferences.getInt(KEY_PUBLISH_INTERVAL, DEFAULT_PUBLISH_INTERVAL);
}

bool Config::isLightEnable()
{
    return preferences.getBool(KEY_LIGHT_ENABLE, true);
}
