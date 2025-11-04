#ifndef WEATHER_UTILITIES_H
#define WEATHER_UTILITIES_H

#include <Arduino.h>
#include <stdint.h> // Include for uint16_t

char getWeatherIcon(String status);
String toTitleCase(String str);
uint16_t getWeatherColor(String status); // <-- ADDED THIS LINE

#endif // WEATHER_UTILITIES_H