#ifndef WEATHERHANDLER_H
#define WEATHERHANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>  // NEW
#include <ArduinoJson.h> // NEW

// --- WEATHER STATE VARIABLES (Declared here, Defined in .ino) ---
// These variables are shared across the project.
extern unsigned long lastWeatherUpdate; 
extern String weatherStatus;
extern float temperatureC; 
extern float humidityPercent;
extern bool weatherDataUpdated; // Signal flag to tell the display to redraw
// --------------------------------------------------------------

// --- FUNCTION PROTOTYPES ---
void fetchWeatherData(); 
void updateWeatherDisplay(); 

#endif // WEATHERHANDLER_H