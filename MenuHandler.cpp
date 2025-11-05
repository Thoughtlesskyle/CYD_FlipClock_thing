#include <TFT_eSPI.h>
#include <WiFi.h>
#include "config.h"      
#include "ThemeConfig.h" 
#include "MenuHandler.h"
#include <Arduino.h> 

// --- FIX FOR WEBSERVER COMPILE ERROR ---
// WebServer.h requires FS.h to be included first,
// and for the 'fs' namespace to be used.
#include <FS.h>
using namespace fs;
#include <WebServer.h>

// --- EXTERN DECLARATIONS from main file (CYD_Flip_Clock_MK7.ino) ---
extern WebServer server;
extern TFT_eSPI tft;
extern unsigned long lastActivityTime;
extern const int DISPLAY_WIDTH;
extern const int DISPLAY_HEIGHT;
extern void checkTouch(int *touchEvent); // Function to check for touch events
extern bool inverted_mode;               // For color toggle logic
extern String timeStringPrevious;        // For redraw logic
extern String dateStringPrevious;        // For redraw logic
extern void setModeColors(bool inverted); // Function to toggle colors
extern uint16_t touchX, touchY;          // Mapped touch coordinates from TouchHandler.h
extern void enterDeepSleep();            // Function to enter deep sleep
extern void performFullReset();          // Function to wipe NVS and reboot
extern void drawStaticElements();        // Function to redraw the colon/layout
extern void drawWeather();               // Function to redraw the weather block
extern void updateTimeDisplay();         // Function to redraw time digits/cards
extern void updateDateDisplay();         // Function to redraw date card
extern void clearWeatherArea();          // <<< NEW EXTERN
extern userConfig_t userConfig;          // Needed to check API key
// NOTE: current_weather_state and weatherStatus are correctly externed via MenuHandler.h
// ------------------------------------------------------------------

// --- MENU TIMEOUT CONSTANT ---
const unsigned long MENU_TIMEOUT_MS = 60000; // 15 seconds

// --- BUTTON DEFINITIONS ---
#define BTN_W 300
#define BTN_H 30
#define BTN_X_START 10
#define BTN_GAP 5

// --- MENU BUTTON COLORS ---
const uint16_t MENU_BTN_COLOR_1 = TFT_SKYBLUE;     // Button 1: Toggle Color Mode
const uint16_t MENU_BTN_COLOR_2 = TFT_ORANGE; // Button 2: IP Configuration
const uint16_t MENU_BTN_COLOR_3 = TFT_CYAN;    // Button 3: Advanced Config
const uint16_t MENU_BTN_COLOR_4 = TFT_RED;       // Button 4: Reboot Device
const uint16_t MENU_BTN_COLOR_5 = TFT_GREEN;    // Button 5: Exit Menu

// Helper function to draw a standard menu button
void drawMenuButton(int buttonIndex, const char* label, uint16_t color, int yStartOffset) {
    // Button indices: 1 (top) to 5 (bottom)
    int yStart = yStartOffset + (buttonIndex - 1) * (BTN_H + BTN_GAP);
    
    // Draw the button body
    tft.fillRoundRect(BTN_X_START, yStart, BTN_W, BTN_H, 5, color);
    // Draw a thin border
    tft.drawRoundRect(BTN_X_START, yStart, BTN_W, BTN_H, 5, TFT_WHITE);

    // Draw the label
    tft.setTextColor(TFT_BLACK, color);
    tft.setTextDatum(MC_DATUM); // Middle-Center datum
    tft.setTextFont(2);
    tft.drawString(label, BTN_X_START + BTN_W / 2, yStart + BTN_H / 2 + 1);
}

// Helper function to check if touch coordinates are within a button's bounds
bool isButtonPressed(int buttonIndex, uint16_t tX, uint16_t tY, int yStartOffset) {
    int yStart = yStartOffset + (buttonIndex - 1) * (BTN_H + BTN_GAP);
    int yEnd = yStart + BTN_H;
    
    return (tX >= BTN_X_START && tX <= (BTN_X_START + BTN_W) &&
            tY >= yStart && tY <= yEnd);
}

// -------------------------------------------------------------
// FUNCTION TO DISPLAY IP CONFIGURATION SCREEN (UPDATED)
// -------------------------------------------------------------

void showIPConfigScreen() {
    
    tft.fillScreen(COLOR_BACKGROUND);
    tft.setTextColor(TFT_WHITE, COLOR_BACKGROUND);
    tft.setTextDatum(MC_DATUM); 
    
    // Header
    tft.setFreeFont(NULL);
    tft.setTextFont(4);
    tft.drawString("IP Configuration", DISPLAY_WIDTH / 2, 20);
    
    // Instructions
    tft.setTextFont(2);
    tft.drawString("Configuration URL:", DISPLAY_WIDTH / 2, 55);
    
    // Display IP Address + /config
    String ipAddress = WiFi.localIP().toString();
    String fullConfigUrl = ipAddress + "/config";

    tft.setTextFont(4); 
    tft.setTextColor(TFT_YELLOW, COLOR_BACKGROUND);
    tft.drawString(fullConfigUrl.c_str(), DISPLAY_WIDTH / 2, 85);
    
    
    // --- STATUS LINES ---
    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, COLOR_BACKGROUND);
    
    // <<< NEW DATUM: Use Top-Right Datum for Right Alignment >>>
    tft.setTextDatum(TR_DATUM); 
    
    // Define constant coordinates for alignment
    const int STATUS_Y_START = 120;
    const int LABEL_X = 10;
    // This is the new, ALIGNED END point for all result values
    const int VALUE_END_X = DISPLAY_WIDTH - 10; // 10 pixels from the right edge
    
    // 1. API Key Status
    bool apiKeyPresent = userConfig.weather_api_key[0] != '\0';
    String apiKeyStatus = apiKeyPresent ? "Yes" : "No";
    uint16_t apiKeyColor = apiKeyPresent ? TFT_GREEN : TFT_RED;
    
    // Switch to TL_DATUM for the labels so they start on the left
    tft.setTextDatum(TL_DATUM); 
    tft.drawString("Weather API Key Present:", LABEL_X, STATUS_Y_START);

    // Switch back to TR_DATUM for the values so they align on the right
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(apiKeyColor, COLOR_BACKGROUND);
    // Draw result ending at the VALUE_END_X coordinate
    tft.drawString(apiKeyStatus, VALUE_END_X, STATUS_Y_START); 
    
    // 2. Location Source Status
    String locationValue;
    uint16_t locationColor;

    if (userConfig.use_city_id_mode) {
        // Using City ID Mode
        if (userConfig.weather_city_id[0] != '\0') {
            locationValue = String(userConfig.weather_city_id);
            locationColor = TFT_GREEN;
        } else {
            locationValue = "ID Not Set";
            locationColor = TFT_RED;
        }
    } else {
        // Using City Name Mode
        if (userConfig.weather_city[0] != '\0') {
            // Using City/Country pair
            locationValue = String(userConfig.weather_city) + ", " + String(userConfig.weather_country_code);
            locationColor = TFT_GREEN;
        } else {
            locationValue = "City Not Set";
            locationColor = TFT_RED;
        }
    }

    // Switch to TL_DATUM for the label
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE, COLOR_BACKGROUND);
    tft.drawString("Location Source:", LABEL_X, STATUS_Y_START + 25);
    
    // Switch back to TR_DATUM for the value
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(locationColor, COLOR_BACKGROUND);
    
    // Draw the location string, also ending at the VALUE_END_X coordinate
    tft.drawString(locationValue, VALUE_END_X, STATUS_Y_START + 25); 
    
    
    // --- Footer ---
    tft.setTextColor(TFT_WHITE, COLOR_BACKGROUND);
    tft.setTextDatum(MC_DATUM); // Back to Middle-Center for the footer
    tft.drawString("Tap to return to menu.", DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 15);

    // --- TOUCH-WAIT LOOP ---
    int touchEvent = 0;
    while (touchEvent == 0) {
        
        // --- FIX: Handle web server requests ---
        // This allows the config page to be loaded while this screen is active.
        // CRITICAL: Handle any incoming web requests (like the / config URL)
        if (WiFi.status() == WL_CONNECTED) {
            server.handleClient();
        }
        // --------------------------------------

        checkTouch(&touchEvent);
        delay(10); // Reduced delay for better web server responsiveness
    }
    
    lastActivityTime = millis();
}

// -------------------------------------------------------------
// Settings Menu Function.
// -------------------------------------------------------------

void showMenu() {
    
    // --- 1. SETUP ---
    tft.fillScreen(COLOR_BACKGROUND);
    tft.setTextColor(TFT_WHITE, COLOR_BACKGROUND);
    tft.setTextDatum(MC_DATUM); 
    tft.setFreeFont(NULL);
    tft.setTextFont(4);
    tft.drawString("Settings Menu", DISPLAY_WIDTH / 2, 20);

    int yOffset = 45; // Start position for buttons

    // --- 2. DRAW BUTTONS (5 buttons total) ---
    // Colors replaced with constants defined at the top of the file
    drawMenuButton(1, "1. IP Configuration", MENU_BTN_COLOR_1, yOffset);
    drawMenuButton(2, "2. Restore to Default Settings", MENU_BTN_COLOR_2, yOffset); 
    drawMenuButton(3, "3. Sleep Now", MENU_BTN_COLOR_3, yOffset);
    drawMenuButton(4, "4. Reboot Device", MENU_BTN_COLOR_4, yOffset);
    drawMenuButton(5, "5. Exit Menu", MENU_BTN_COLOR_5, yOffset); 

// --- 3. INPUT LOOP (UPDATED WITH TIMEOUT) ---
    bool menuActive = true;
    int touchEvent = 0;
    
    // Store the time when the menu was opened
    unsigned long menuStartTime = millis();

    while (menuActive) {

        // CRITICAL: Handle any incoming web requests (like the / config URL)
        if (WiFi.status() == WL_CONNECTED) {
            server.handleClient();
        }
        
        // Check for any touch event
        checkTouch(&touchEvent);
        
        // --- TIMEOUT CHECK ---
        if (millis() - menuStartTime > MENU_TIMEOUT_MS) {
            Serial.println("[MENU] Timeout reached. Returning to clock.");
            menuActive = false; // Exit the loop
            break; // Exit the loop immediately
        }
        // ---------------------
        
        if (touchEvent != 0) {
            
            // A touch was registered, reset the menu timer
            menuStartTime = millis();
            
            // --- 1. IP Configuration ---
            if (isButtonPressed(1, touchX, touchY, yOffset)) {
                Serial.println("[MENU] Button 1: IP Configuration pressed.");
                showIPConfigScreen(); 
                showMenu(); 
                return; 
            }

            // --- 2. Restore to Default Settings ---
            else if (isButtonPressed(2, touchX, touchY, yOffset)) {
                Serial.println("[MENU] Button 2: Restore to Default Settings pressed.");
                performFullReset(); 
                return; 
            }
            
            // --- 3. Sleep Now (Immediate Deep Sleep) ---
            else if (isButtonPressed(3, touchX, touchY, yOffset)) {
                Serial.println("[MENU] Button 3: Sleep Now pressed. Entering Deep Sleep...");
                tft.fillScreen(COLOR_BACKGROUND);
                tft.setTextColor(TFT_WHITE, COLOR_BACKGROUND);
                tft.setTextFont(4);
                tft.drawString("Going to Sleep...", DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2);
                delay(500);
                enterDeepSleep(); 
                return; 
            }

            // --- 4. Reboot Device ---
            else if (isButtonPressed(4, touchX, touchY, yOffset)) {
                Serial.println("[MENU] Button 4: Reboot Device pressed. Rebooting...");
                tft.fillScreen(COLOR_BACKGROUND);
                tft.setTextColor(TFT_RED, COLOR_BACKGROUND);
                tft.setTextFont(4);
                tft.drawString("REBOOTING...", DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2);
                delay(1000);
                ESP.restart();
            }
            
            // --- 5. Exit Menu ---
            else if (isButtonPressed(5, touchX, touchY, yOffset)) {
                Serial.println("[MENU] Button 5: Exit Menu pressed.");
                menuActive = false;
            }
            
            // Clear the touch event
            touchEvent = 0;
        }
        delay(50); 
    }
    
    // --- 4. EXIT MENU (Common cleanup) ---
    tft.fillScreen(COLOR_BACKGROUND);
    
    // Set previous strings to force a full redraw of dynamic elements BEFORE calling draw functions
    timeStringPrevious = "XX:XX"; 
    dateStringPrevious = "XX XXX XXXX";
    
    drawStaticElements(); 
    updateTimeDisplay();  
    updateDateDisplay();  
    
    // Conditional draw on exit
    if (current_weather_state == WEATHER_OK) {
        drawWeather();
    } else {
        clearWeatherArea();
    }

    lastActivityTime = millis(); // Reset sleep timer after exiting
}