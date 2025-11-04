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

// === REMOVED: Weather Icon Color Toggle ===
// This is now controlled by 'userConfig.use_multi_color_icons'
// static const bool USE_MULTI_COLOR_ICONS = true; // <-- THIS LINE IS REMOVED

// === NEW: Multi-Color Icon Definitions ===
// Add/change any colors you like here
static const uint16_t COLOR_ICON_SUN     = TFT_YELLOW;
static const uint16_t COLOR_ICON_MOON    = 0x751C;     // A darker blue/cyan
static const uint16_t COLOR_ICON_RAIN    = 0x64BD;     // Light Blue
static const uint16_t COLOR_ICON_THUNDER = TFT_ORANGE;
static const uint16_t COLOR_ICON_CLOUDS  = TFT_LIGHTGREY;
static const uint16_t COLOR_ICON_FOG     = 0x9CD3;     // A light mist gray
static const uint16_t COLOR_ICON_SNOW    = TFT_WHITE;
static const uint16_t COLOR_ICON_DEFAULT = TFT_RED;      // For unknown weather

// --- Single Color Fallback ---
const uint16_t COLOR_ICON_NORMAL = TFT_LIGHTGREY;      // Icon Color for Dark Mode
const uint16_t COLOR_ICON_INVERTED = TFT_LIGHTGREY;    // Icon Color for Light Mode

// Weather Description Text Color
const uint16_t COLOR_WEATHER_TEXT_NORMAL = TFT_LIGHTGREY; 
const uint16_t COLOR_WEATHER_TEXT_INVERTED = TFT_DARKGREY; 

#endif // THEME_CONFIG_H