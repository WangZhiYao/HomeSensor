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
    if (doc.containsKey(KEY_COLLECT_INTERVAL))
    {
        int collectInterval = doc[KEY_COLLECT_INTERVAL].as<int>();
        Serial.print("Set collect interval to: ");
        Serial.println(collectInterval);
        preferences.putInt(KEY_PREF_COLLECT_INTERVAL, collectInterval);
    }
    if (doc.containsKey(KEY_COLLECT_ILLUMINANCE))
    {
        bool collectIlluminance = doc[KEY_COLLECT_ILLUMINANCE].as<bool>();
        Serial.print("Set collect illuminance to: ");
        Serial.println(collectIlluminance);
        preferences.putBool(KEY_PREF_COLLECT_ILLUMINANCE, collectIlluminance);
    }
}

int Config::getCollectInterval()
{
    return preferences.getInt(KEY_PREF_COLLECT_INTERVAL, DEFAULT_COLLECT_INTERVAL);
}

bool Config::collectIlluminance()
{
    return preferences.getBool(KEY_PREF_COLLECT_ILLUMINANCE, true);
}
