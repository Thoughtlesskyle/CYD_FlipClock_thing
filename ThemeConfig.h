#ifndef THEME_CONFIG_H
#define THEME_CONFIG_H

#include <TFT_eSPI.h> 

// --- 1. FONT FILE INCLUSIONS ---
#include "meteocons10pt7b.h" 

// --- 2. FONT DEFINITIONS (Assigning specific font pointers) ---
#define CLOCK_FONT  NULL 
#define DATE_FONT_CUSTOM NULL 
#define WEATHER_FONT_CUSTOM NULL 
#define WEATHER_ICON_FONT &meteocons10pt7b // The selected icon font


// --- 3. BASE COLORS ---
const uint16_t COLOR_BACKGROUND = TFT_BLACK;

// --- 4. CLOCK & CARD COLORS (Dynamic) ---
// Dark Mode (Normal)
const uint16_t COLOR_DARK_TEXT = TFT_WHITE;     
const uint16_t COLOR_CARD_NORMAL = 0x2104;      // Dark Blue/Gray
const uint16_t COLOR_COLON_NORMAL = TFT_WHITE;  // Colon/Separator color in Dark Mode

// Light Mode (Inverted)
const uint16_t COLOR_LIGHT_TEXT = 0x2104;       
const uint16_t COLOR_CARD_INVERTED = TFT_WHITE; 
const uint16_t COLOR_COLON_INVERTED = TFT_WHITE;   // Colon/Separator color in Light Mode

// --- 5. WEATHER COLORS (Dynamic) ---
// Icon Color
const uint16_t COLOR_ICON_NORMAL = TFT_YELLOW;      // Icon Color for Dark Mode
const uint16_t COLOR_ICON_INVERTED = TFT_YELLOW;    // Icon Color for Light Mode

// Weather Description Text Color
const uint16_t COLOR_WEATHER_TEXT_NORMAL = TFT_LIGHTGREY; 
const uint16_t COLOR_WEATHER_TEXT_INVERTED = TFT_DARKGREY; 

#endif // THEME_CONFIG_H