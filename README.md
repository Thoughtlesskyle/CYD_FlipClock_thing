# CYD_FlipClock_thing
An over engineered flip clock inspired design for the CYD.


After playing around with the CYD, I thought it might be fun to have something that looked like a flip clock (recovering HTC lover) that I could have on my desk. 
As I was vibe coding this, I kept getting ideas for new features but still largely wanted to keep the idea simple. A clock, that you can tap on the screen to change from dark mode to light mode.

Features: 
- Dark Mode and Light Mode clocks accessible by a single tap on the screen
- A Simple settings menu accessible by a double tap
- Long press to turn off the backlight
- A Minimal Weather display giving current conditions.
- An Included Web interface accessible from a web browser on the same network (the exact address is in the settings menu under the IP configuration options)
- A minimal web based initial setup

Web Interface Features
- Support for Openweather API key for weather data
- Configurable Location Via Location name for Location ID (defaults to New York)
- Ability to Toggle between Fahrenheit/Celsius
- Ability to choose between Multi-Color and Monochrome weather icons
- 
- Ability to choose between 12/24 hour clocks
- Configurable Time Zone (defaults to -5GMT EST)
- Configurable Sleep Timeout (this is a deep sleep of the clock, a tap on the physical screen will be required to wake the device up)
- Save Settings Button
- Button to Toggle Backlight
- Button to Deep Sleep the Device
- Button to Reboot the Device

Required Libraries:
TFT_eSPI             Bodmer      
XPT2046_Touchscreen  Paul Stoffregen
ArduinoJson          Benoît Blanchon
WiFiManager          tzapu


![PXL_20251030_200838457](https://github.com/user-attachments/assets/4d43d2de-9f2c-477b-ba15-0a0b6a27c89f)
![PXL_20251104_152031378](https://github.com/user-attachments/assets/dd80bd77-06c6-46aa-9786-d03fa2e0f5cc)

*The gradient/checkered pattern effect on the screen is from trying to take a picture it's not how it looks in person 

