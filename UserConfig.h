#ifndef USERCONFIG_H
#define USERCONFIG_H

// --- CONFIGURATION MACROS ---
#define PREF_NAMESPACE "FlipClock" 

// 2. DEFINE THE CONFIGURATION STRUCTURE
// IMPORTANT: Changes to this struct require re-flashing the ESP32 to clear old NVS data 
// if you experience unexpected behavior or size mismatches.
typedef struct {
    // WiFi Credentials (saved by WiFiManager)
    char ssid[32];
    char password[64];
    
    // Core Settings
    char weather_api_key[40];
    int gmt_offset_hr;
    int sleep_timeout_min;
    
    // >>> NEW: TIME FORMAT CONFIGURATION <<<
    // true (1) = 24-Hour format, false (0) = 12-Hour format
    bool time_format_24h;
    
    // >>> ADDED: TEMPERATURE UNIT CONFIGURATION <<<
    // true (1) = Fahrenheit, false (0) = Celsius
    bool use_fahrenheit; // <--- ADDED LINE
    
    // Location Mode Toggle
    // true (1) = Use City ID search method (id=...)
    // false (0) = Use City Name search method (q=...)
    bool use_city_id_mode;      
    
    // Location Details (Used when use_city_id_mode is false)
    char weather_city[40];       // e.g., "San Francisco, CA"
    char weather_country_code[4]; // e.g., "US"
    
    // Location ID (Used when use_city_id_mode is true)
    char weather_city_id[12];    // e.g., "5391959" (for San Jose, CA)
    
} userConfig_t;

// 3. DECLARE THE GLOBAL INSTANCE
extern userConfig_t userConfig;

#endif // USERCONFIG_H