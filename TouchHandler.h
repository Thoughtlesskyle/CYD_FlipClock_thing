#ifndef TOUCHHANDLER_H
#define TOUCHHANDLER_H

#include <Arduino.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h> 

// --- EXTERNAL DEPENDENCIES ---
extern const int DISPLAY_WIDTH;
extern const int DISPLAY_HEIGHT;
// -----------------------------

// --- OBJECT DEFINITIONS (Required for linker) ---
SPIClass touchSPI(VSPI); 
XPT2046_Touchscreen ts(TS_CS, TS_IRQ); 
// -----------------------------------------------------------------

// --- FINAL TOUCH CALIBRATION VALUES (Tested and Confirmed) ---
const uint16_t X_MIN_RAW = 1000; 
const uint16_t X_MAX_RAW = 4000; 
const uint16_t Y_MIN_RAW = 250;  // ADJUSTED FROM 245 to 250
const uint16_t Y_MAX_RAW = 4800; 
// ----------------------------------------------------

// --- Touch Detection Variables ---
const long DEBOUNCE_DELAY_MS = 45;     
const long DOUBLE_CLICK_TIME_MS = 600; 
const long LONG_PRESS_TIME_MS = 2000;  // 2 seconds for long press

uint16_t touchX = 0, touchY = 0;       
bool isTouched = false;                
unsigned long lastPressTime = 0;       
unsigned long touchStartTime = 0;      // To track press duration
int pressCount = 0;                    

/**
 * @brief Checks for touch events, handling debouncing and click counting.
 * @param touchEvent A pointer to the global touchEvent integer (1=single, 2=double, 3=long press).
 */
void checkTouch(int *touchEvent) {

    bool currentTouched = ts.touched();
    
    if (currentTouched && !isTouched) {
        delay(DEBOUNCE_DELAY_MS);
        currentTouched = ts.touched();
    }
    
    if (currentTouched) {
        if (!isTouched) {
            isTouched = true;
            touchStartTime = millis(); // Start tracking press duration
        }
        
        // Read raw point data
        TS_Point p = ts.getPoint();
        
        // --- FINAL FIX LOGIC: 90-degree swap + X-axis flip (Correct Order) ---

        // 1. RAW DATA TRANSLATION
        // Calculate screen Y (320-dimension) from the raw Y data (p.y).
        // Y-axis is NOT inverted. Range adjusted via Y_MAX_RAW/Y_MIN_RAW.
        uint16_t tempX = map(p.y, Y_MIN_RAW, Y_MAX_RAW, 0, DISPLAY_WIDTH); 

        // Calculate screen X (240-dimension) from the raw X data (p.x).
        // X-axis IS INVERTED (MAX -> MIN). Range adjusted via X_MIN_RAW/X_MAX_RAW.
        uint16_t tempY = map(p.x, X_MAX_RAW, X_MIN_RAW, 0, DISPLAY_HEIGHT); 

        // 2. FINAL ASSIGNMENT: Swap the results to fix the 90-degree rotation.
        touchX = tempY; // Screen X (0-240) gets the X calculation (inverted)
        touchY = tempX; // Screen Y (0-320) gets the Y calculation (adjusted range)
        
        // Clamp
        if (touchX > DISPLAY_WIDTH) touchX = DISPLAY_WIDTH;
        if (touchY > DISPLAY_HEIGHT) touchY = DISPLAY_HEIGHT;
        if (touchX < 0) touchX = 0;
        if (touchY < 0) touchY = 0;
    } else {
        if (isTouched) { 
            isTouched = false;
            unsigned long currentTime = millis();
            
            // <<< LONG PRESS DETECTION >>>
            if (currentTime - touchStartTime >= LONG_PRESS_TIME_MS) {
                *touchEvent = 3; 
                pressCount = 0;     
                lastPressTime = 0;  
                return;             
            }
            // <<< END LONG PRESS DETECTION >>>
            
            if (currentTime - lastPressTime < DOUBLE_CLICK_TIME_MS) {
                pressCount++;
            } else {
                pressCount = 1;
            }
            lastPressTime = currentTime; 
        }
    }

    if (pressCount > 0 && (millis() - lastPressTime > DOUBLE_CLICK_TIME_MS)) {
        
        if (pressCount == 1) {
            *touchEvent = 1; // Single Press
        } else if (pressCount >= 2) {
            *touchEvent = 2; // Double Press (or more)
        }
        
        // Reset counters after a touch event is registered
        pressCount = 0;
        lastPressTime = 0;
    }
}

#endif // TOUCHHANDLER_H