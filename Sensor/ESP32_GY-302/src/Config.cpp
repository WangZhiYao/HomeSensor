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

int Config::getPublishInterval()
{
    return preferences.getInt(KEY_PUBLISH_INTERVAL, DEFAULT_PUBLISH_INTERVAL);
}

bool Config::setPublishInterval(int publishInterval)
{
    return preferences.putInt(KEY_PUBLISH_INTERVAL, publishInterval) != 0;
}
