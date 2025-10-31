#include <WiFi.h>
#include <HTTPClient.h> 
#include <ArduinoJson.h> 
#include <time.h>
#include <TFT_eSPI.h> 
#include <esp_sleep.h> 
#include <SPI.h> 
#include <XPT2046_Touchscreen.h> 
#include <WiFiClientSecure.h> 
#include <Preferences.h>      // Include Preferences for NVS

// CRITICAL: FS.h must be included before WebServer.h to prevent compiler errors
#include <FS.h>
using namespace fs;
// ------------------------------------------------------------------------------------

// --- WIFI MANAGER INCLUDES ---
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h> 
// -----------------------------

#include "config.h"      
#include "TouchHandler.h"
#include "SleepHandler.h"
#include "WeatherUtilities.h" 
#include "ThemeConfig.h"      
#include "UserConfig.h"       
#include "WebConfig.h" 
#include "MenuHandler.h" 

// --- EXTERN DECLARATIONS FOR TOUCH OBJECTS ---
extern SPIClass touchSPI;
extern XPT2046_Touchscreen ts;
extern void checkTouch(int *touchEvent);
// ------------------------------------------------------------------

// --- GLOBAL CONSTANTS DEFINITIONS ---
const int LED_PIN = 21;
const int DISPLAY_WIDTH = 320;
const int DISPLAY_HEIGHT = 240;
// ----------------------------------------------------------------

// ------------------------------------
// 2. GLOBAL VARIABLES & DISPLAY SETUP
// ------------------------------------

TFT_eSPI tft = TFT_eSPI();

String timeStringCurrent = "00:00";
String timeStringPrevious = "XX:XX"; // Sentinel value to force full redraw on first loop
String dateStringCurrent = "";
String dateStringPrevious = "XX XXX XXXX"; // Sentinel value to force full redraw on first loop

int touchEvent = 0;
unsigned long lastActivityTime = 0; // Used for sleep timer
bool backlight_state = true; // Tracks the backlight (LED_PIN) state
// ------------------------------------------------

// --- DYNAMIC COLOR STATE ---
static bool inverted_mode = false; // Tracks the current color theme (Normal/Inverted)
uint16_t current_digit_color = COLOR_DARK_TEXT;
uint16_t current_card_color = COLOR_CARD_NORMAL;
uint16_t current_colon_color = COLOR_COLON_NORMAL;
uint16_t current_icon_color = COLOR_ICON_NORMAL;
uint16_t current_weather_text_color = COLOR_WEATHER_TEXT_NORMAL;
// ------------------------------------------------

// --- WIFI MANAGER CUSTOM PARAMETERS ---
// Temporary buffers for WiFiManager to read default values
char api_key_param[40];
char gmt_offset_param[6]; 
char sleep_timeout_param[6];
char time_format_param[2]; 
char city_param[40];
char country_param[4];
char id_param[12]; 

// Pointers to the WiFiManager parameter objects
WiFiManagerParameter *custom_api_key_field;
WiFiManagerParameter *custom_gmt_offset_field;
WiFiManagerParameter *custom_sleep_timeout_field;
WiFiManagerParameter *custom_time_format_field;
WiFiManagerParameter *custom_city_field; 
WiFiManagerParameter *custom_country_field; 
WiFiManagerParameter *custom_id_field;
// --------------------------------------

// --- DIMENSIONS & WEATHER VARIABLES ---

const int DIGIT_WIDTH = 62;
const int COLON_WIDTH = 30;
const int DIGIT_GAP = 4;
const int COLON_X_ADJUSTMENT = -3; // Fine-tuning for colon horizontal position
const int MAIN_TIME_WIDTH = (4 * DIGIT_WIDTH) + COLON_WIDTH + (3 * DIGIT_GAP);
const int DIGIT_HEIGHT = 100;
const int CARD_RADIUS = 10;
const int DATE_HEIGHT = 40;
const int DATE_VERTICAL_GAP = 7;
const int DATE_FONT_BUILTIN = 4;
const int DATE_Y_ADJUSTMENT = 3; // Fine-tuning for date vertical position
const int TOTAL_BLOCK_HEIGHT = DIGIT_HEIGHT + DATE_VERTICAL_GAP + DATE_HEIGHT;
const int Y_OFFSET = ((DISPLAY_HEIGHT - TOTAL_BLOCK_HEIGHT) / 2) - 10; // Main Y-offset for the clock/date block, adjusted 10px up
const int DATE_Y_OFFSET = Y_OFFSET + DIGIT_HEIGHT + DATE_VERTICAL_GAP;

float temperature = 0.0;
float humidityPercent = 0.0;
String weatherStatus = "Fetching...";
unsigned long lastWeatherUpdate = 0;
bool weatherDataUpdated = false; // Flag to trigger a redraw of the weather
String temperatureUnit = " "; // Will be set to "C" or "F"

// --- NEW WEATHER STATUS TRACKING ---
// Global definition for the weather status state machine.
// The type 'WeatherState' is defined in MenuHandler.h.
WeatherState current_weather_state = WEATHER_DISABLED;
// -----------------------------------


// ------------------------------------
// 3. CORE FUNCTIONS PROTOTYPES
// ------------------------------------
void setupTime();
void drawSegment(const char* text, int xPos, int yPos, int width, uint16_t color, bool isCard);
void drawStaticElements();
void updateTimeDisplay();
void updateDateDisplay();
String manualUrlEncode(const String& str);
void fetchWeatherData();
void drawWeather();     
void setModeColors(bool inverted);
void saveConfigCallback();
void startConfigPortal();
void clearWeatherArea(); 
void toggleBacklight();   // Toggles LED_PIN high/low
void performFullReset();  // Wipes all NVS settings and reboots
// ----------------------------------------------------------------
// --- CONFIGURATION MANAGEMENT ---

// Define the global configuration instance (INSTANTIATION)
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
// ----------------------------------------------------------------


// ------------------------------------
// 4. FUNCTION IMPLEMENTATIONS 
// ------------------------------------

/**
 * @brief Sets all global color variables based on the inverted mode.
 * @param inverted True for light-on-dark, False for dark-on-light.
 */
void setModeColors(bool inverted) {
    inverted_mode = inverted;
    if (inverted_mode) {
        current_digit_color = COLOR_LIGHT_TEXT;
        current_card_color = COLOR_CARD_INVERTED;
        current_colon_color = COLOR_COLON_INVERTED;
        current_icon_color = COLOR_ICON_INVERTED;
        current_weather_text_color = COLOR_WEATHER_TEXT_INVERTED;
    } else {
        current_digit_color = COLOR_DARK_TEXT;
        current_card_color = COLOR_CARD_NORMAL;
        current_colon_color = COLOR_COLON_NORMAL;
        current_icon_color = COLOR_ICON_NORMAL;
        current_weather_text_color = COLOR_WEATHER_TEXT_NORMAL;
    }
}

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
    userConfig.time_format_24h = atoi(custom_time_format_field->getValue()) == 1; // 1 -> true, 0 -> false
    
    // 2. Copy location fields
    strncpy(userConfig.weather_city, custom_city_field->getValue(), sizeof(userConfig.weather_city) - 1);
    strncpy(userConfig.weather_country_code, custom_country_field->getValue(), sizeof(userConfig.weather_country_code) - 1);
    strncpy(userConfig.weather_city_id, custom_id_field->getValue(), sizeof(userConfig.weather_city_id) - 1);
    
    // 3. AUTOMATIC MODE SWITCH 
    // If the City ID field is NOT empty, use City ID mode
    if (strlen(userConfig.weather_city_id) > 0) {
        userConfig.use_city_id_mode = true;
        Serial.println("Weather Mode set to: City ID.");
    } else {
        // If the City ID field IS empty, use City Name mode
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
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM); 
    
    tft.setFreeFont(NULL); // Use default fonts
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
    
    // Clean up memory
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

/**
 * @brief Connects to WiFi and synchronizes time with NTP server.
 * Launches config portal if no WiFi is saved or connection fails.
 */
void setupTime() {
    const long gmtOffset_sec = userConfig.gmt_offset_hr * 3600;
    const int daylightOffset_sec = DST_ACTIVE ? 3600 : 0;
    
    // If no Wi-Fi config is saved, start the portal immediately.
    if (userConfig.ssid[0] == '\0') {
        Serial.println("No Wi-Fi config saved. Starting Configuration Portal immediately.");
        startConfigPortal();
        setupTime(); // Recursive call to retry time setup
        return;
    }
    
    Serial.print("Attempting Wi-Fi connection...");
    tft.fillRect(0, 0, DISPLAY_WIDTH, 25, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("Connecting...", 5, 5, 2);

    WiFi.begin(userConfig.ssid, userConfig.password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected.");
        Serial.print("Local IP Address: ");
        Serial.println(WiFi.localIP()); 
        tft.fillRect(0, 0, DISPLAY_WIDTH, 25, TFT_DARKGREEN);
        tft.drawString("WiFi OK.", 5, 5, 2);
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        Serial.println("Time synchronized with NTP.");
    } else {
        Serial.println("\nWiFi failed. Starting Configuration Portal.");
        startConfigPortal();
        setupTime(); // Recursive call to retry time setup
        return;
    }
    
    delay(1500); // Show "WiFi OK" message briefly
}

/**
 * @brief Draws a single time segment (digit or colon).
 * @param text The character to draw (e.g., "8", ":").
 * @param xPos Left X coordinate.
 * @param yPos Top Y coordinate.
 * @param width Width of the segment.
 * @param color Text color.
 * @param isCard True = draw rounded rect card; False = draw transparent (for colon).
 */
void drawSegment(const char* text, int xPos, int yPos, int width, uint16_t color, bool isCard) {
    
    if (isCard) {
        // Draw the rounded rectangle "card"
        tft.fillRoundRect(xPos, yPos, width, DIGIT_HEIGHT, CARD_RADIUS, current_card_color);
    } else {
        // Erase the background (for the colon)
        tft.fillRect(xPos, yPos, width, DIGIT_HEIGHT, COLOR_BACKGROUND);
    }

    if (text[0] != '\0') {
        // Set text color and background (transparent if not a card)
        uint16_t textBgColor = isCard ? current_card_color : COLOR_BACKGROUND;
        tft.setTextColor(color, textBgColor); 
        tft.setTextDatum(MC_DATUM); // Middle-Center datum
        
        if (CLOCK_FONT != NULL) {
            tft.setFreeFont(CLOCK_FONT);
        } else {
            tft.setTextFont(7); // Fallback to a large built-in font
        }
        
        int centerX = xPos + width / 2;
        if (!isCard) {
            centerX += COLON_X_ADJUSTMENT; // Apply fine-tuning for colon
        }

        int centerY = Y_OFFSET + DIGIT_HEIGHT / 2;
        tft.drawString(text, centerX, centerY);
    }
}

/**
 * @brief Draws the static colon element.
 */
void drawStaticElements() {
    // Calculate the X position of the colon: (Start) + (Digit 1) + (Gap 1) + (Digit 2) + (Gap 2)
    int x_start = (DISPLAY_WIDTH - MAIN_TIME_WIDTH) / 2;
    if (x_start < 0) x_start = 0; 
    
    int colon_x = x_start + (2 * DIGIT_WIDTH) + (2 * DIGIT_GAP);
    
    drawSegment(":", colon_x, Y_OFFSET, COLON_WIDTH, current_colon_color, false);
}

/**
 * @brief Updates the time display, only redrawing digits that have changed.
 */
void updateTimeDisplay() {
    
    int x = (DISPLAY_WIDTH - MAIN_TIME_WIDTH) / 2;
    if (x < 0) x = 0; 
    
    // Iterate over all 5 characters (HH:MM)
    for (int i = 0; i < 5; i++) {
        
        bool hasChanged = timeStringCurrent[i] != timeStringPrevious[i];
        bool isColon = timeStringCurrent[i] == ':';
        int segmentWidth = isColon ? COLON_WIDTH : DIGIT_WIDTH;
        
        // Only redraw digits (not the colon) if they have changed
        if (!isColon) {
             if (hasChanged || timeStringPrevious == "XX:XX") { // "XX:XX" forces full redraw
                String digitStr = String(timeStringCurrent[i]);
                drawSegment(digitStr.c_str(), x, Y_OFFSET, segmentWidth, current_digit_color, true);
             } 
        }
        
        x += segmentWidth;
        // Add a gap after H1, H2, and M1 (indices 0, 1, 3)
        if (i == 0 || i == 1 || i == 3) {
             x += DIGIT_GAP;
        }
    }
}

/**
 * @brief Updates the date display if the date string has changed.
 */
void updateDateDisplay() {
    
    if (dateStringCurrent != dateStringPrevious || dateStringPrevious == "XX XXX XXXX" || weatherDataUpdated) {
        
        int cardWidth = MAIN_TIME_WIDTH;
        int cardX = (DISPLAY_WIDTH - cardWidth) / 2;
        if (cardX < 0) cardX = 0;
        
        // Draw the date "card"
        tft.fillRoundRect(cardX, DATE_Y_OFFSET, cardWidth, DATE_HEIGHT, CARD_RADIUS, current_card_color);
        
        tft.setTextColor(current_digit_color, current_card_color);
        tft.setTextDatum(MC_DATUM);
        
        if (DATE_FONT_CUSTOM != NULL) {
            tft.setFreeFont(DATE_FONT_CUSTOM);
        } else {
            tft.setTextFont(DATE_FONT_BUILTIN); // Fallback
        }
        
        int centerX = cardX + cardWidth / 2;
        int centerY = DATE_Y_OFFSET + DATE_HEIGHT / 2 + DATE_Y_ADJUSTMENT;
        
        tft.drawString(dateStringCurrent.c_str(), centerX, centerY);
        Serial.print("Date updated to: ");
        Serial.println(dateStringCurrent);
    }
}

/**
 * @brief Draws the weather icon and text, centered on the screen.
 */
void drawWeather() {
    // Only proceed if weather data is valid (i.e., not an error state)
    if (current_weather_state != WEATHER_OK) return;
    
    char icon = getWeatherIcon(weatherStatus);
    
    // 'temperature' is already in the correct unit (C or F) from fetchWeatherData
    String tempDisplay = String((int)round(temperature));
    tempDisplay += temperatureUnit;
    
    String statusTitleCase = toTitleCase(weatherStatus);
    String combinedWeather = statusTitleCase;
    
    // Truncate long weather descriptions
    if (combinedWeather.length() > 18) {
        combinedWeather = combinedWeather.substring(0, 17) + "...";
    }
    combinedWeather += " - " + tempDisplay;

    const int TEXT_VERTICAL_ADJUSTMENT = -7;
    const int ICON_TEXT_GAP = 20;            
    
    // Clear the drawing area regardless
    clearWeatherArea();
    
    int weatherYCenter = DATE_Y_OFFSET + DATE_HEIGHT + 5 + 25; 
    int screenXCenter = DISPLAY_WIDTH / 2;
    
    // --- Dynamically center the icon + text block ---
    tft.setFreeFont(NULL);
    tft.setTextFont(2);
    int textWidth = tft.textWidth(combinedWeather);
    
    tft.setFreeFont(WEATHER_ICON_FONT);
    int iconWidth = tft.textWidth(String(icon));
    
    int totalWidth = iconWidth + ICON_TEXT_GAP + textWidth;
    int blockXStart = screenXCenter - (totalWidth / 2);
    // --- End of centering logic ---

    // 1. Draw Icon
    int iconXCenter = blockXStart + (iconWidth / 2);
    tft.setFreeFont(WEATHER_ICON_FONT);
    tft.setTextColor(current_icon_color, COLOR_BACKGROUND); 
    tft.setTextDatum(MC_DATUM);
    tft.drawChar(icon, iconXCenter, weatherYCenter);
    
    // 2. Draw Text
    int textXCenter = iconXCenter + (iconWidth / 2) + ICON_TEXT_GAP + (textWidth / 2);
    tft.setFreeFont(NULL);
    tft.setTextFont(2);
    tft.setTextColor(current_weather_text_color, COLOR_BACKGROUND);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(combinedWeather, textXCenter, weatherYCenter + TEXT_VERTICAL_ADJUSTMENT);
    
    Serial.printf("Weather Display updated: %c %s\n", icon, combinedWeather.c_str());
}


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
            encodedString += '+'; // Common for query strings
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

// ----------------------------------------------------------------
// UTILITY FUNCTIONS IMPLEMENTATIONS 
// ----------------------------------------------------------------

/**
 * @brief Clears the weather display area (bottom part of the screen).
 */
void clearWeatherArea() {
    int clearYStart = DATE_Y_OFFSET + DATE_HEIGHT + 5;
    tft.fillRect(0, clearYStart, DISPLAY_WIDTH, 70, COLOR_BACKGROUND);
    Serial.println("Weather area cleared.");
}

/**
 * @brief Toggles the display backlight ON/OFF.
 * If turning ON, forces a full screen redraw.
 */
void toggleBacklight() {
    backlight_state = !backlight_state;
    if (backlight_state) {
        // --- Turn ON ---
        digitalWrite(LED_PIN, HIGH);
        // Treat turning on as activity to prevent immediate sleep
        lastActivityTime = millis(); 
        
        tft.fillScreen(COLOR_BACKGROUND); 
        
        // Force a full redraw of time and date by resetting previous values
        timeStringPrevious = "XX:XX";
        dateStringPrevious = "XX XXX XXXX";
        
        drawStaticElements(); // Redraw colon
        updateTimeDisplay();  // Will now redraw all digits
        updateDateDisplay();  // Will now redraw date card
        
        if (current_weather_state == WEATHER_OK) {
            drawWeather(); // Redraw weather if state is OK
        } else {
            clearWeatherArea(); // Ensure area is blank if in error
        }
        
        Serial.println("Backlight ON.");
    } else {
        // --- Turn OFF ---
        digitalWrite(LED_PIN, LOW);
        Serial.println("Backlight OFF.");
    }
}

/**
 * @brief Performs a full factory reset by wiping NVS and rebooting the device.
 */
void performFullReset() {
    Serial.println("Menu Action: Configuration Reset initiated.");
    
    // 1. Display Warning / Confirmation Message
    tft.fillScreen(COLOR_BACKGROUND);
    tft.setTextColor(TFT_RED, COLOR_BACKGROUND);
    tft.setTextDatum(MC_DATUM); 
    tft.setFreeFont(NULL);
    tft.setTextFont(4);
    tft.drawString("WIPING ALL SETTINGS", DISPLAY_WIDTH / 2, 100);
    tft.setTextFont(2); 
    tft.drawString("Rebooting to start Configuration Portal...", DISPLAY_WIDTH / 2, 140);
    
    // 2. Wipe WiFiManager's stored credentials
    WiFiManager wm;
    wm.resetSettings();
    Serial.println("WiFiManager Settings Wiped.");
    
    // 3. Delete ALL saved settings in our custom namespace
    preferences.begin(PREF_NAMESPACE, false);
    preferences.clear();
    preferences.end();
    Serial.println("NVS Config Cleared.");
    
    delay(3000);
    ESP.restart(); 
}

// ------------------------------------
// 7. ARDUINO SETUP AND LOOP
// ------------------------------------

void setup() {
    Serial.begin(115200);
    
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) {
        Serial.println("Woke up from deep sleep (Touch Event)");
    } else {
        Serial.println("Power-on or Reset");
    }
    
    lastActivityTime = millis(); // Initialize activity time
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // Turn backlight on at boot
    
    loadConfig(); // CRITICAL: Load configuration immediately

    tft.init();
    tft.setRotation(1); 
    tft.fillScreen(COLOR_BACKGROUND);
    
    // Initialize Touchscreen
    touchSPI.begin(TS_CLK, TS_MISO, TS_MOSI, -1);
    ts.begin(touchSPI);
    
    setupTime(); // Connect to WiFi and get NTP time

    // Start Web Server for Direct IP Configuration (if connected)
    if (WiFi.status() == WL_CONNECTED) {
        startConfigServer();
    }

    setModeColors(false); // Set initial theme to normal

    tft.fillScreen(COLOR_BACKGROUND); 
    
    drawStaticElements(); // Draw the colon
    fetchWeatherData(); // Initial fetch sets current_weather_state

    if (current_weather_state == WEATHER_OK) {
        drawWeather();
    } else {
        clearWeatherArea(); // Don't draw weather if it failed
    }

    // Set sentinel values to force initial draw in loop()
    timeStringPrevious = "XX:XX";
    timeStringCurrent = "";
    dateStringPrevious = "XX XXX XXXX";
    dateStringCurrent = "";
}

void loop() {
    
    // Handle any incoming web requests (like the /config URL)
    if (WiFi.status() == WL_CONNECTED) {
        server.handleClient();
    }
    
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time. Retrying WiFi/NTP setup.");
        delay(1000);
        setupTime(); // Attempt to re-establish connection and time
        tft.fillScreen(COLOR_BACKGROUND); // Clear screen for redraw
        return; // Skip this loop iteration
    }
    
    // Format the current time string
    char timeBuffer[6];
    const char *timeFormat = userConfig.time_format_24h ? "%R" : "%I:%M"; // %R = HH:MM (24h), %I:%M = hh:MM (12h)
    strftime(timeBuffer, 6, timeFormat, &timeinfo);
    timeStringCurrent = String(timeBuffer);
    
    // Format the current date string
    char dateBuffer[20];
    const char *dateFormat = "%a, %b %d, %Y"; // e.g., "Mon, Sep 30, 2024"
    strftime(dateBuffer, 20, dateFormat, &timeinfo);
    dateStringCurrent = String(dateBuffer);
    
    // Update display elements (only redraws if changed)
    updateTimeDisplay();
    updateDateDisplay();
    
    // Fetch weather data (function handles its own timing)
    fetchWeatherData();
    
    if (weatherDataUpdated) {
        if (current_weather_state == WEATHER_OK) {
            drawWeather(); // Draw new weather
        } else {
            clearWeatherArea(); // Clear old weather if fetch failed
        }
        weatherDataUpdated = false; // Reset flag
    }

    // --- Handle Touch Events ---
    checkTouch(&touchEvent);
    if (touchEvent != 0) {
        lastActivityTime = millis(); // Any touch resets the sleep timer
    }

    if (touchEvent == 1) {
        // === Single Press: Color Toggle ===
        
        // Only toggle colors if the backlight is ON
        if (backlight_state) { 
            setModeColors(!inverted_mode);
            
            // Force full redraw
            tft.fillScreen(COLOR_BACKGROUND);
            drawStaticElements();
            timeStringPrevious = "XX:XX";
            dateStringPrevious = "XX XXX XXXX";
            updateTimeDisplay();
            updateDateDisplay();
            
            // Redraw weather conditionally
            if (current_weather_state == WEATHER_OK) {
                drawWeather();
            } else {
                clearWeatherArea();
            }
            
            Serial.print("Main Loop Action: Single Press - Color mode toggled to ");
            Serial.println(inverted_mode ? "INVERTED" : "NORMAL");
        } else {
            // If screen is off, a single press just wakes it up
            toggleBacklight();
        }
        
        touchEvent = 0;
        
    } else if (touchEvent == 2) {
        // === Double Press: Open Settings Menu ===
        
        Serial.println("Main Loop Action: Double Press - Opening Settings Menu.");
        if (!backlight_state) {
            toggleBacklight(); // Wake up screen first
        }
        
        showMenu(); // Show the main settings menu
        
        // Menu function handles redrawing the screen on exit
        touchEvent = 0;
        
    } else if (touchEvent == 3) { 
        // === Long Press: Backlight Toggle ===
        
        Serial.println("Main Loop Action: Long Press - Backlight Toggle.");
        toggleBacklight(); // Toggles screen on or off
        
        touchEvent = 0;
    }

    // --- Check for Deep Sleep ---
    // Only check if sleep timeout is enabled (greater than 0)
    if (userConfig.sleep_timeout_min > 0 && (millis() - lastActivityTime > userConfig.sleep_timeout_min * 60L * 1000L)) {
        enterDeepSleep();
    } 

    // --- End of loop housekeeping ---
    timeStringPrevious = timeStringCurrent;
    dateStringPrevious = dateStringCurrent;
    delay(100); // Small delay to prevent spamming
} 