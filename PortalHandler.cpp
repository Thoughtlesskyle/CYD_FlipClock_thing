#include "PortalHandler.h"
#include "ConfigHandler.h"   // For extern userConfig and saveConfig()
#include "WebPortalHtml.h"   // For HTML strings (used in original .ino logic)
#include "MenuHandler.h"     // For externs of tft, DISPLAY_WIDTH, DISPLAY_HEIGHT

#include <Arduino.h>         // For Serial.println and strncpy
#include <WiFi.h>            // For WiFi.SSID/psk
#include <TFT_eSPI.h>        // For tft.fillScreen, tft.drawString, etc.

// --- EXTERN GLOBALS (from .ino) ---
// Note: We access these via MenuHandler.h's extern declarations
extern TFT_eSPI tft;
extern const int DISPLAY_WIDTH;
extern const int DISPLAY_HEIGHT;

// --- INSTANTIATION OF WIFI MANAGER CUSTOM PARAMETERS (Globals defined here) ---
char api_key_param[40];
char gmt_offset_param[6]; 
char sleep_timeout_param[6];
char time_format_param[2]; 
char city_param[40];
char country_param[4];
char id_param[12]; 

WiFiManagerParameter *custom_api_key_field = nullptr;
WiFiManagerParameter *custom_gmt_offset_field = nullptr;
WiFiManagerParameter *custom_sleep_timeout_field = nullptr;
WiFiManagerParameter *custom_time_format_field = nullptr;
WiFiManagerParameter *custom_city_field = nullptr; 
WiFiManagerParameter *custom_country_field = nullptr; 
WiFiManagerParameter *custom_id_field = nullptr;


/**
 * @brief Callback function triggered by WiFiManager when "Save" is pressed.
 * Saves settings from form fields back into the userConfig struct.
 */
void saveConfigCallback() {
    Serial.println("WiFiManager save config callback called.");
    
    // 1. Copy mandatory data
    strncpy(userConfig.weather_api_key, custom_api_key_field->getValue(), sizeof(userConfig.weather_api_key) - 1);
    userConfig.gmt_offset_hr = atoi(custom_gmt_offset_field->getValue());
    userConfig.sleep_timeout_min = atoi(custom_sleep_timeout_field->getValue());
    userConfig.time_format_24h = atoi(custom_time_format_field->getValue()) == 1; 
    
    // 2. Copy location fields
    strncpy(userConfig.weather_city, custom_city_field->getValue(), sizeof(userConfig.weather_city) - 1);
    strncpy(userConfig.weather_country_code, custom_country_field->getValue(), sizeof(userConfig.weather_country_code) - 1);
    strncpy(userConfig.weather_city_id, custom_id_field->getValue(), sizeof(userConfig.weather_city_id) - 1);
    
    // 3. AUTOMATIC MODE SWITCH 
    if (strlen(userConfig.weather_city_id) > 0) {
        userConfig.use_city_id_mode = true;
        Serial.println("Weather Mode set to: City ID.");
    } else {
        userConfig.use_city_id_mode = false;
        Serial.println("Weather Mode set to: City Name.");
    }
    
    saveConfig(); // Save all changes to NVS
}


/**
 * @brief Starts the blocking WiFiManager Configuration Portal.
 * Displays instructions on the TFT.
 */
void startConfigPortal() {
    
    // Display portal instructions on the screen
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM); 
    
    tft.setFreeFont(NULL);
    tft.setTextFont(4);
    tft.drawString("Configuration Portal", DISPLAY_WIDTH / 2, 40);
    tft.setTextFont(2);
    tft.drawString("1. Connect to network:", DISPLAY_WIDTH / 2, 80);
    tft.drawString("FlipClockSetup", DISPLAY_WIDTH / 2, 105);
    tft.drawString("2. Go to 192.168.4.1", DISPLAY_WIDTH / 2, 130);
    tft.drawString("3. Enter Wi-Fi/Settings", DISPLAY_WIDTH / 2, 160);
    
    WiFiManager wm;

    wm.setSaveConfigCallback(saveConfigCallback);
    
    // Populate parameter buffers with current config values
    snprintf(api_key_param, sizeof(api_key_param), "%s", userConfig.weather_api_key);
    snprintf(gmt_offset_param, sizeof(gmt_offset_param), "%d", userConfig.gmt_offset_hr);
    snprintf(sleep_timeout_param, sizeof(sleep_timeout_param), "%d", userConfig.sleep_timeout_min);
    snprintf(time_format_param, sizeof(time_format_param), "%d", userConfig.time_format_24h ? 1 : 0);
    snprintf(city_param, sizeof(city_param), "%s", userConfig.weather_city);
    snprintf(country_param, sizeof(country_param), "%s", userConfig.weather_country_code);
    snprintf(id_param, sizeof(id_param), "%s", userConfig.weather_city_id);

    // Create parameters
    custom_api_key_field = new WiFiManagerParameter("apikey", "OpenWeatherMap API Key", api_key_param, 40);
    custom_gmt_offset_field = new WiFiManagerParameter("gmt", "GMT Offset (e.g., -5)", gmt_offset_param, 6);
    custom_sleep_timeout_field = new WiFiManagerParameter("sleeptmo", "Sleep Timeout (min)", sleep_timeout_param, 6);
    custom_time_format_field = new WiFiManagerParameter("timefmt", "Time Format (0=12h, 1=24h)", time_format_param, 2);
    custom_city_field = new WiFiManagerParameter("city", "City (e.g. New York)", city_param, 40);
    custom_country_field = new WiFiManagerParameter("country", "Country Code (e.g. US)", country_param, 4);
    custom_id_field = new WiFiManagerParameter("city_id", "City ID (if using ID mode, leave city/country blank)", id_param, 12);
    
    // Add parameters to WiFiManager
    wm.addParameter(custom_api_key_field);
    wm.addParameter(custom_gmt_offset_field);
    wm.addParameter(custom_sleep_timeout_field);
    wm.addParameter(custom_time_format_field);
    wm.addParameter(custom_city_field);
    wm.addParameter(custom_country_field);
    wm.addParameter(custom_id_field);
    
    // Start the portal
    if (!wm.autoConnect("FlipClockSetup", NULL)) {
        Serial.println("Failed to connect or configure. Resetting.");
        delay(3000);
        ESP.restart();
    }
    
    Serial.println("Connected to Wi-Fi. Portal exited.");
    
    // Clean up memory (CRITICAL)
    delete custom_api_key_field;
    delete custom_gmt_offset_field;
    delete custom_sleep_timeout_field;
    delete custom_time_format_field;
    delete custom_city_field;
    delete custom_country_field;
    delete custom_id_field;

    // Save final Wi-Fi details (in case they changed)
    strncpy(userConfig.ssid, WiFi.SSID().c_str(), sizeof(userConfig.ssid) - 1);
    strncpy(userConfig.password, WiFi.psk().c_str(), sizeof(userConfig.password) - 1);
    saveConfig();
}