#ifndef SLEEPHANDLER_H
#define SLEEPHANDLER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <esp_sleep.h> 
#include "TouchHandler.h" // Needed for TS_IRQ pin definition

// --- GLOBAL VARIABLES DECLARED EXTERNALLY IN .INO ---
extern TFT_eSPI tft;
extern const int DISPLAY_WIDTH;
extern const int DISPLAY_HEIGHT;
extern const int LED_PIN;               
extern int touchEvent;
extern const uint16_t COLOR_BACKGROUND; 
// ----------------------------------------------------

// --- SLEEP MODE CONTROLS ---
extern unsigned long lastActivityTime; // <--- FIX: Declared extern, defined in .ino

/**
 * @brief Prepares the ESP32 for deep sleep, setting the touch IRQ pin as the wake source.
 */
void enterDeepSleep() {
    Serial.println("Entering deep sleep...");
    
    // 1. Turn off the backlight and clear screen
    digitalWrite(LED_PIN, LOW);
    tft.fillScreen(COLOR_BACKGROUND); 

    // 2. Clear all touch variables
    touchEvent = 0; 

    // 3. Configure the external wake-up source
    const uint64_t wakeUpPinMask = (1ULL << TS_IRQ);
    esp_sleep_enable_ext1_wakeup(wakeUpPinMask, ESP_EXT1_WAKEUP_ALL_LOW);

    // 4. Enter Deep Sleep
    esp_deep_sleep_start();
}

#endif // SLEEPHANDLER_H