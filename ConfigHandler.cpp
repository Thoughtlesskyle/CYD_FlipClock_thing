#include "ConfigHandler.h"
#include "UserConfig.h" // For userConfig_t and PREF_NAMESPACE
#include "config.h"     // For default values like TIME_FORMAT_24H, USE_FAHRENHEIT

#include <Arduino.h>    // For Serial.println/printf and strncpy

// --- INSTANTIATION OF GLOBALS ---
// These variables are defined here (ConfigHandler.cpp)
userConfig_t userConfig;
Preferences preferences;
#define CONFIG_KEY "userConfig"


/**
 * @brief Loads the configuration struct from NVS.
 */
void loadConfig() {
    preferences.begin(PREF_NAMESPACE, true); // Read-only mode 
    size_t bytesRead = preferences.getBytes(CONFIG_KEY, &userConfig, sizeof(userConfig));
    preferences.end();
    
    if (bytesRead == 0 || bytesRead != sizeof(userConfig)) {
        Serial.println("No saved config found or size mismatch. Setting fresh install defaults.");
        // Set all critical defaults here
        userConfig.gmt_offset_hr = -5; // Default to New York time zone
        userConfig.sleep_timeout_min = 5; // Default sleep timeout to 5 minutes
        
        userConfig.time_format_24h = TIME_FORMAT_24H;
        userConfig.use_fahrenheit = USE_FAHRENHEIT;
        
        userConfig.use_multi_color_icons = true; // <-- ADDED DEFAULT

        userConfig.use_city_id_mode = false;
        // Set default location: New York, US
        strncpy(userConfig.weather_city, "New York", sizeof(userConfig.weather_city) - 1);
        strncpy(userConfig.weather_country_code, "US", sizeof(userConfig.weather_country_code) - 1);

        // Ensure API key and Wi-Fi credentials are empty to trigger the config portal
        userConfig.weather_api_key[0] = '\0';
        userConfig.ssid[0] = '\0';
        userConfig.password[0] = '\0';

    } else {
        Serial.printf("Config loaded successfully (%u bytes).\n", bytesRead);
    }
}

/**
 * @brief Saves the configuration struct to NVS.
 */
void saveConfig() {
    preferences.begin(PREF_NAMESPACE, false); // Read/Write mode
    size_t bytesWritten = preferences.putBytes(CONFIG_KEY, &userConfig, sizeof(userConfig));
    preferences.end();
    Serial.printf("Configuration saved to NVS. Bytes written: %u\n", bytesWritten);
}