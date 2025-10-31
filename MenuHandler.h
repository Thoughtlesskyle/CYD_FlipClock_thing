#ifndef MENUHANDLER_H
#define MENUHANDLER_H

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <Preferences.h>
#include "UserConfig.h"   // For userConfig_t struct
#include "config.h"       // For PREF_NAMESPACE and LED_PIN
#include "ThemeConfig.h"  // For COLOR_BACKGROUND

// Forward declarations of external objects/variables needed by the menu
extern TFT_eSPI tft;
extern XPT2046_Touchscreen ts;

// Fix 1: Use the correct type name 'userConfig_t'
extern userConfig_t userConfig; 

extern bool backlight_state;
extern unsigned long lastActivityTime;

// FIX 3: Add extern declarations for missing time/date strings
extern String timeStringPrevious;
extern String dateStringPrevious; 

// Fix 2: Add extern declarations for missing constants
extern const int LED_PIN; 
extern const int DISPLAY_WIDTH;
extern const int DISPLAY_HEIGHT;

// --- NEW: Weather Status Type Definition and Extern Declarations ---
// The ENUM TYPE is defined here ONLY.
typedef enum { 
    WEATHER_OK, 
    WEATHER_NO_KEY, 
    WEATHER_ERROR, 
    WEATHER_DISABLED // Initial state before first fetch
} WeatherState;

// The global variables are declared as extern.
extern WeatherState current_weather_state; 
extern String weatherStatus;
// ------------------------------------------------------------------

/**
 * @brief Handles the drawing of the main settings menu and touch interaction.
 *
 * This function is called on a double-press event. It takes over control,
 * draws the menu, and waits for a button press before returning.
 */
void showMenu();

#endif // MENUHANDLER_H