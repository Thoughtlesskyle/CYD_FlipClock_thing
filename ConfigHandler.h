#ifndef CONFIGHANDLER_H
#define CONFIGHANDLER_H

#include <Preferences.h>
#include "UserConfig.h" // Defines userConfig_t

// --- GLOBAL EXTERN DECLARATIONS ---
// The global configuration struct instance (defined in .cpp)
extern userConfig_t userConfig; 
// The NVS preferences object (defined in .cpp)
extern Preferences preferences;

// --- FUNCTION PROTOTYPES ---
void loadConfig();
void saveConfig();

#endif // CONFIGHANDLER_H