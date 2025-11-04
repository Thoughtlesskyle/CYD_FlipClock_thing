#ifndef WEATHERHANDLER_H
#define WEATHERHANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>  
#include <ArduinoJson.h> 

// --- WEATHER STATE VARIABLES (Declared here, Defined in .ino) ---
// These variables are shared across the project.
extern unsigned long lastWeatherUpdate; 
extern String weatherStatus;
extern float temperature;       // <-- FIXED (was temperatureC)
extern float humidityPercent;
extern bool weatherDataUpdated; // Signal flag to tell the display to redraw
extern String temperatureUnit;  // <-- ADDED

// --- FUNCTION PROTOTYPES ---
void fetchWeatherData(); 
void updateWeatherDisplay(); // This prototype was already here

#endif // WEATHERHANDLER_H