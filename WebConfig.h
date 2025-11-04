#ifndef WEBCONFIG_H
#define WEBCONFIG_H

#include <WebServer.h>
#include "UserConfig.h" 
#include "config.h"     // For PREF_NAMESPACE and external settings
#include "WebPortalHtml.h" 

// --- EXTERNAL DEPENDENCIES ---
extern userConfig_t userConfig; 
extern void saveConfig();       
extern void enterDeepSleep();   
extern void toggleBacklight();   
extern bool backlight_state;     
// ----------------------------

// --- SERVER INSTANTIATION ---
WebServer server(80); 
// ----------------------------


/**
 * @brief Sends the configuration form and handles the submission/saving of user settings.
 */
// In: WebConfig.h

/**
 * @brief Sends the configuration form and handles the submission/saving of user settings.
 */
void handleConfig() {
    
    // --- 1. HANDLE SUBMITTED DATA (GET request parameters) ---
    if (server.hasArg("apikey")) {
        
        Serial.println("Web Server: Saving configuration from direct IP page.");

        // --- FIX: Use temporary String variables to safely trim and use input ---
        
        // 1. API Key
        String tempApiKey = server.arg("apikey");
        tempApiKey.trim();
        strncpy(userConfig.weather_api_key, tempApiKey.c_str(), sizeof(userConfig.weather_api_key) - 1);
        
        // 2. Numeric/Boolean Settings
        userConfig.gmt_offset_hr = server.arg("gmt").toInt();
        userConfig.sleep_timeout_min = server.arg("sleeptmo").toInt();
        
        // Save Time Format
        userConfig.time_format_24h = server.arg("timefmt").toInt() == 1; 
        
        // Save Temperature Unit
        userConfig.use_fahrenheit = server.arg("tempunit").toInt() == 1;
        
        // --- NEW: Save Icon Color Setting ---
        userConfig.use_multi_color_icons = server.arg("iconcolor").toInt() == 1;
        
        // 3. Location Data
        String tempCity = server.arg("city");
        tempCity.trim();
        strncpy(userConfig.weather_city, tempCity.c_str(), sizeof(userConfig.weather_city) - 1);

        String tempCountry = server.arg("country");
        tempCountry.trim();
        strncpy(userConfig.weather_country_code, tempCountry.c_str(), sizeof(userConfig.weather_country_code) - 1);
        
        // 4. City ID Data
        String tempCityID = server.arg("city_id");
        tempCityID.trim();
        strncpy(userConfig.weather_city_id, tempCityID.c_str(), sizeof(userConfig.weather_city_id) - 1);
        
        // 5. Location Mode Toggle
        userConfig.use_city_id_mode = (tempCityID.length() > 0); 
        
        // Save the updated configuration
        saveConfig();
    
        server.send(200, "text/plain", "Settings saved successfully! Restarting Flip Clock to apply changes...");
        delay(500);
        ESP.restart();
        return; 
    }
    
    // --- 2. GENERATE HTML RESPONSE ---
    
    // CRITICAL FIX: Declare the 'html' string variable here!
    String html = HTML_HEAD; 

    // Determine selected temperature unit
    String selectedF = userConfig.use_fahrenheit ? "selected" : "";
    String selectedC = userConfig.use_fahrenheit ? "" : "selected";

    // Determine selected time format
    String selected24 = userConfig.time_format_24h ? "selected" : "";
    String selected12 = userConfig.time_format_24h ? "" : "selected";

    // --- NEW: Determine selected icon color format ---
    String selectedMulti = userConfig.use_multi_color_icons ? "selected" : "";
    String selectedSingle = userConfig.use_multi_color_icons ? "" : "selected";

    
    // Start of form
    html += HTML_FORM_START;
    html += WiFi.SSID(); 

    // API Key Field
    html += HTML_WEATHER_START;
    html += "'";
    html += userConfig.weather_api_key;
    html += "'";
    
    // City Name, Country Code, and City ID Fields
    html += HTML_LOCATION_FIELDS;
    html += "'";
    html += userConfig.weather_city;
    html += "'";
    
    html += HTML_COUNTRY_FIELD;
    html += "'";
    html += userConfig.weather_country_code;
    html += "'";
    
    html += HTML_CITY_ID_FIELD;
    html += "'";
    html += userConfig.weather_city_id;
    html += "'";
    
    // Location Notes
    html += HTML_LOCATION_NOTES;

    // Weather Units Section
    html += R"raw(<h3>Weather Units</h3><label for='tempunit'>Temperature Unit:</label><select id='tempunit' name='tempunit'>)raw";
    html += "<option value='1' " + selectedF + ">Fahrenheit</option>";
    html += "<option value='0' " + selectedC + ">Celsius</option>";
    html += "</select><br>";
    
    // --- NEW: Icon Color Section ---
    html += R"raw(<label for='iconcolor'>Weather Icon Style:</label><select id='iconcolor' name='iconcolor'>)raw";
    html += "<option value='1' " + selectedMulti + ">Multi-Color</option>";
    html += "<option value='0' " + selectedSingle + ">Monochrome </option>";
    html += "</select><br>";


    // Time Format Select
    html += HTML_TIME_START;
    html += "<option value='0' " + selected12 + ">12-Hour (AM/PM)</option>";
    html += "<option value='1' " + selected24 + ">24-Hour</option>";
    
    // GMT Offset
    html += HTML_GMT_START;
    html += "'" + String(userConfig.gmt_offset_hr) + "'"; 
    
    // Sleep Timeout and Footer
    html += HTML_GMT_END_SLEEP_START;
    html += "'" + String(userConfig.sleep_timeout_min) + "'"; 
    html += HTML_FOOTER;
    
    server.send(200, "text/html", html);
}

/**
 * @brief Handle root request, redirects to config.
 */
void handleRoot() {
    server.sendHeader("Location", "/config");
    server.send(302, "text/plain", "Redirecting to /config");
}

/**
 * @brief Handle Reboot request.
 */
void handleReboot() {
    server.send(200, "text/plain", "Rebooting...");
    delay(500);
    ESP.restart();
}

/**
 * @brief Handle Deep Sleep request.
 */
void handleDeepSleep() {
    server.send(200, "text/plain", "Entering deep sleep...");
    delay(500);
    enterDeepSleep();
}

/**
 * @brief Handle Backlight Toggle request.
 */
void handleBacklightToggle() {
    toggleBacklight(); // Call the external function
    // FIX: Explicitly cast the string literal to a String object to enable concatenation
    String status = String("Backlight is now ") + (backlight_state ? "ON" : "OFF");
    server.send(200, "text/plain", status);
}


/**
 * @brief Sets up server routing and starts the HTTP server.
 */
void startConfigServer() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/config", HTTP_GET, handleConfig);
    server.on("/reboot", HTTP_GET, handleReboot);
    server.on("/sleep", HTTP_GET, handleDeepSleep);
    server.on("/toggle_backlight", HTTP_GET, handleBacklightToggle); 
    
    server.begin();
    Serial.println("HTTP Config Server started.");
}

#endif // WEBCONFIG_H