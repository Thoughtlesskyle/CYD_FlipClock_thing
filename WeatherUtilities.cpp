#include "WeatherUtilities.h"
#include <time.h> // Needed for getLocalTime() for day/night icon check

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
    if (status.indexOf("mist") != -1 || status.indexOf("fog") != -1) return 'M'; // Mist/Fog
    
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
    if (status.indexOf("few clouds") != -1) return 'E';       // Few Clouds
    if (status.indexOf("overcast clouds") != -1) return 'S';  // Overcast
    if (status.indexOf("clouds") != -1) return 'N';           // Generic Cloud
    
    return '!'; // Default (Exclamation mark)
}