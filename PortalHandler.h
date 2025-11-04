#ifndef PORTALHANDLER_H
#define PORTALHANDLER_H

#include <WiFiManager.h>
#include <WebServer.h>

// --- EXTERN DECLARATIONS FOR WIFI MANAGER VARIABLES ---
// These char arrays are buffers used by WiFiManager
extern char api_key_param[40];
extern char gmt_offset_param[6]; 
extern char sleep_timeout_param[6];
extern char time_format_param[2]; 
extern char city_param[40];
extern char country_param[4];
extern char id_param[12]; 

// These are pointers to the WiFiManager parameter objects
extern WiFiManagerParameter *custom_api_key_field;
extern WiFiManagerParameter *custom_gmt_offset_field;
extern WiFiManagerParameter *custom_sleep_timeout_field;
extern WiFiManagerParameter *custom_time_format_field;
extern WiFiManagerParameter *custom_city_field; 
extern WiFiManagerParameter *custom_country_field; 
extern WiFiManagerParameter *custom_id_field;

// --- FUNCTION PROTOTYPES ---
void startConfigPortal();
void saveConfigCallback();

#endif // PORTALHANDLER_H