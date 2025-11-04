#include "WeatherUtilities.h"
#include <time.h> // Needed for getLocalTime() for day/night icon check
#include "ThemeConfig.h" // Gives access to the new color constants

// Helper function to convert a string to Title Case
String toTitleCase(String str) {
    if (str.length() == 0) return str;
    // Convert the entire string to lowercase first
    str.toLowerCase();
    // Capitalize the first letter
    str.setCharAt(0, toupper(str.charAt(0)));
    // Iterate through the string and capitalize any letter following a space
    for (int i = 1; i < str.length(); i++) {
        if (str.charAt(i - 1) == ' ' && str.charAt(i) != ' ') {
            str.setCharAt(i, toupper(str.charAt(i)));
        }
    }
    return str;
}


// Function to map OpenWeatherMap status to Meteocons character
char getWeatherIcon(String status) {
    status.toLowerCase();

    // Need to get current time for day/night icon check
    struct tm timeinfo;
    getLocalTime(&timeinfo); 

    if (status.indexOf("thunderstorm") != -1) return 'r'; // Thunder
    if (status.indexOf("drizzle") != -1) return 'Q';      // Drizzle
    if (status.indexOf("rain") != -1) return 'R';         // Rain
    if (status.indexOf("snow") != -1) return 'W';         // Snow
    // --- UPDATED: Added || status.indexOf("haze") != -1 ---
    if (status.indexOf("mist") != -1 || status.indexOf("fog") != -1 || status.indexOf("haze") != -1) return 'M'; // Mist/Fog/Haze
    
    // Check for clear/sun
    if (status.indexOf("clear sky") != -1) {
        // Simple check for day/night (assumes 6am-8pm is day)
        if (timeinfo.tm_hour >= 6 && timeinfo.tm_hour < 20) {
            return 'B'; // Sun (Day)
        } else {
            return 'C'; // Moon (Night)
        }
    }
    
    // Check for clouds
    if (status.indexOf("broken clouds") != -1) return 'Y'; // Broken Clouds
    if (status.indexOf("scattered clouds") != -1) return 'H'; // Scattered Clouds
    if (status.indexOf("few clouds") != -1) return 'H'; // Few Clouds
    if (status.indexOf("overcast clouds") != -1) return 'N'; // Overcast Clouds
    if (status.indexOf("cloudy") != -1) return 'N'; // General Cloudy

    // Default icon (question mark)
    return ')'; 
}


/**
 * @brief Maps OpenWeatherMap status to a specific icon color.
 * @param status The weather description string.
 * @return The 16-bit color (uint16_t) for the icon.
 */
uint16_t getWeatherColor(String status) {
    status.toLowerCase();

    // Need to get current time for day/night icon check
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        // Use a default (daytime) if time isn't available
        timeinfo.tm_hour = 12; 
    }

    if (status.indexOf("thunderstorm") != -1) return COLOR_ICON_THUNDER;
    if (status.indexOf("drizzle") != -1) return COLOR_ICON_RAIN;
    if (status.indexOf("rain") != -1) return COLOR_ICON_RAIN;
    if (status.indexOf("snow") != -1) return COLOR_ICON_SNOW;
    // --- UPDATED: Added || status.indexOf("haze") != -1 ---
    if (status.indexOf("mist") != -1 || status.indexOf("fog") != -1 || status.indexOf("haze") != -1) return COLOR_ICON_FOG;
    
    // Check for clear/sun
    if (status.indexOf("clear sky") != -1) {
        // Simple check for day/night (assumes 6am-8pm is day)
        if (timeinfo.tm_hour >= 6 && timeinfo.tm_hour < 20) {
            return COLOR_ICON_SUN; // Sun (Day)
        } else {
            return COLOR_ICON_MOON; // Moon (Night)
        }
    }
    
    // Check for clouds (all types)
    if (status.indexOf("clouds") != -1) return COLOR_ICON_CLOUDS;
    
    // Return a default error color if nothing matches
    return COLOR_ICON_DEFAULT; 
}