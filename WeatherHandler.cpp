#include "WeatherHandler.h"   // Header for this module
#include "config.h"           // For constants (OPENWEATHER_URL_BASE, etc.)
#include "UserConfig.h"       // For the userConfig struct
#include "MenuHandler.h"      // For WeatherState enum and externs

// Include libraries needed for implementation
#include <WiFiClientSecure.h>

// Note: HTTPClient.h, ArduinoJson.h, WiFi.h, Arduino.h
// are already included via WeatherHandler.h


/**
 * @brief Custom URL encoder for City Names. Required for OWM API calls.
 * @param str The string to encode (e.g., "New York").
 * @return The URL-encoded string (e.g., "New+York").
 */
String manualUrlEncode(const String& str) {
    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        // Standard safe characters: alphanumeric, -._~
        if (c == ' ') {
            encodedString += '+';
            // Common for query strings
        } else if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encodedString += c;
        } else {
            // Hex encoding logic
            code0 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code0 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code1 = c + '0';
            if (c > 9) {
                code1 = c - 10 + 'A';
            }
            encodedString += '%';
            encodedString += code1;
            encodedString += code0;
        }
    }
    return encodedString;
}


/**
 * @brief Fetches weather data using the configured method (City Name or City ID).
 * Updates global weather variables and sets the current_weather_state.
 */
void fetchWeatherData() {
    
    // Throttle updates
    if (lastWeatherUpdate != 0 && (millis() - lastWeatherUpdate < WEATHER_UPDATE_INTERVAL_MS)) {
        return;
    }
    
    // Sanity checks: Do not attempt fetch if config is missing.
    if (userConfig.weather_api_key[0] == '\0') {
        Serial.println("FATAL ERROR: OpenWeatherMap API Key is empty. Skipping weather fetch.");
        weatherStatus = "No API Key";
        current_weather_state = WEATHER_NO_KEY;
        lastWeatherUpdate = millis();
        return;
    }
    if (!userConfig.use_city_id_mode && (userConfig.weather_city[0] == '\0' || userConfig.weather_country_code[0] == '\0')) {
        Serial.println("FATAL ERROR: Using City Name mode, but City or Country code is empty. Skipping.");
        weatherStatus = "No City Config";
        current_weather_state = WEATHER_ERROR;
        lastWeatherUpdate = millis();
        return;
    }
    if (userConfig.use_city_id_mode && userConfig.weather_city_id[0] == '\0') {
        Serial.println("FATAL ERROR: Using City ID mode, but City ID is empty. Skipping.");
        weatherStatus = "No ID Config";
        current_weather_state = WEATHER_ERROR;
        lastWeatherUpdate = millis();
        return;
    }
    
    String oldWeatherStatus = weatherStatus;
    float oldTemperature = temperature;
    String oldTemperatureUnit = temperatureUnit;
    Serial.println("--- Attempting to fetch weather data ---");
    
    if (WiFi.status() == WL_CONNECTED) {
        
        WiFiClientSecure client;
        client.setInsecure(); // Allow HTTPS connections without a root CA certificate
        HTTPClient http;
        String url = OPENWEATHER_URL_BASE;
        
        if (userConfig.use_city_id_mode) {
            // Mode 1: Use City ID
            url += "id=";
            url += userConfig.weather_city_id;
            Serial.printf("Weather Source: ID (%s)\n", userConfig.weather_city_id); 
        } else {
            // Mode 2: Use City Name (URL-Encoding required)
            url += "q=";
            String cityUrlEncoded = manualUrlEncode(userConfig.weather_city);
            
            url += cityUrlEncoded;
            url += ",";
            url += userConfig.weather_country_code;
            Serial.printf("Weather Source: Location (%s, %s)\n", userConfig.weather_city, userConfig.weather_country_code);
        }
        
        // Append Units based on user configuration
        if (userConfig.use_fahrenheit) {
            url += "&units=imperial";
            temperatureUnit = "F"; 
        } else {
            url += "&units=metric";
            temperatureUnit = "C"; 
        }
        
        url += "&appid=";
        url += userConfig.weather_api_key; 

        //Serial.print("Final URL: "); // Useful for debugging
        //Serial.println(url); 

        http.begin(client, url);
        int httpResponseCode = http.GET();
        
        if (httpResponseCode == 200) {
            String payload = http.getString();
            StaticJsonDocument<3000> doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
                float temp = doc["main"]["temp"];
                const char* description = doc["weather"][0]["description"];
                
                temperature = temp;
                weatherStatus = String(description); 
                current_weather_state = WEATHER_OK;
                Serial.println("Weather data received successfully.");
            } else {
                Serial.print("JSON Parsing FAILED: ");
                Serial.println(error.f_str());
                weatherStatus = "JSON Error";
                current_weather_state = WEATHER_ERROR;
            }
            
        } else {
            Serial.printf("HTTP GET Failed, Code: %d. Error: %s\n", httpResponseCode, http.errorToString(httpResponseCode).c_str());
            if (httpResponseCode == 404) {
                 weatherStatus = "Location Not Found";
            } else if (httpResponseCode == 401) {
                 weatherStatus = "Invalid API Key";
            } else {
                 weatherStatus = "HTTP Error";
            }
            current_weather_state = WEATHER_ERROR;
        }
        
        http.end();
    } else {
        weatherStatus = "WiFi Offline";
        current_weather_state = WEATHER_ERROR;
    }

    // Flag for redraw only if data has actually changed
    if (weatherStatus != oldWeatherStatus || abs(temperature - oldTemperature) > 0.1 || temperatureUnit != oldTemperatureUnit) {
        weatherDataUpdated = true;
    }
    
    lastWeatherUpdate = millis();
}