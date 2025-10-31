#ifndef WEBPORTALHTML_H
#define WEBPORTALHTML_H

// --- 1. HTML HEAD & STYLE ---
const char* HTML_HEAD = R"raw(
<!DOCTYPE html>
<html>
<head>
    <title>Clock Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body{font-family:Arial,sans-serif;margin:20px;}
        input[type=text],input[type=number],select{width:100%;padding:10px;margin:8px 0;box-sizing:border-box;}
        input[type=submit]{background-color:#4CAF50;color:white;padding:14px 20px;margin:8px 0;border:none;cursor:pointer;width:100%;}
        button{background-color:#f44336;color:white;padding:14px 20px;margin:8px 0;border:none;cursor:pointer;width:100%;}
    </style>
</head>
<body>
)raw";

// --- 2. CONFIG FORM START (WiFi SSID is inserted here) ---
const char* HTML_FORM_START = R"raw(
<h2>Flip Clock Settings</h2>
<p>Connected to WiFi: )raw";

// --- 3. WEATHER SETTINGS START (API Key is inserted here) ---
// Note: Ends right after value=
const char* HTML_WEATHER_START = R"raw(
</p>
<form method='get' action='config'>
<h3>OpenWeatherMap Settings</h3>
<label for='apikey'>API Key:</label>
<input type='text' id='apikey' name='apikey' value=)raw";

// --- 4. LOCATION FIELDS (City/Country/ID are inserted here) ---
// Note: Requires closing quote after API key value
const char* HTML_LOCATION_FIELDS = R"raw(
 maxlength='39'><br>
<p style='font-size: 0.9em; color: #555;'>*If you do not have an API Key visit openweathermap.org to create an account.
</p> 
<label for='city'>City Name (e.g., London):</label>
<input type='text' id='city' name='city' value=)raw";

// Note: Requires closing quote after City value
const char* HTML_COUNTRY_FIELD = R"raw(
 maxlength='39'><br>
<label for='country'>Country Code (e.g., GB):</label>
<input type='text' id='country' name='country' value=)raw";

// Note: Requires closing quote after Country value
const char* HTML_CITY_ID_FIELD = R"raw(
 maxlength='3'><br>
<label for='city_id'>City ID (Optional):</label>
<input type='text' id='city_id' name='city_id' value=)raw";

const char* HTML_LOCATION_NOTES = R"raw(
 maxlength='11'><br>
<p style='font-size: 0.9em; color: #555;'>*If City ID is entered, the clock will use ID mode and ignore City/Country fields.</p>
)raw";

// --- 5. TIME & SLEEP SETTINGS START (Time Format options are inserted here) ---
const char* HTML_TIME_START = R"raw(
<h3>Time and Sleep</h3>
<label for='timefmt'>Time Format:</label>
<select id='timefmt' name='timefmt'>
)raw"; 

// --- 5b. TEMPERATURE FORMAT START (Temperature Format options are inserted here)
const char* HTML_TEMP_START = R"raw(
</select><br>
<label for='tempfmt'>Temperature Format:</label>
<select id='tempfmt' name='tempfmt'>
)raw"; 

// --- 6. GMT FIELD START (Requires closing select tag and opening quote for value=) ---
const char* HTML_GMT_START = R"raw(
</select><br>
<label for='gmt'>GMT Offset (Hours, e.g., -5 or 1):</label>
<input type='number' id='gmt' name='gmt' value=)raw";

// --- 7. SLEEP FIELD START (Closes GMT input, starts Sleep input, requires quotes) ---
const char* HTML_GMT_END_SLEEP_START = R"raw(
><br>
<label for='sleeptmo'>Sleep Timeout (Minutes, 0 to disable):</label>
<input type='number' id='sleeptmo' name='sleeptmo' value=)raw";


// --- 8. HTML FOOTER (Closes Sleep input with quotes, adds submit/buttons) ---
const char* HTML_FOOTER = R"raw(
><br>
<input type='submit' value='Save Settings'>
</form>
<form method='get' action='deep_sleep'><button>Deep Sleep</button></form>
<form method='get' action='reboot'><button style='background-color:#1e90ff;'>Reboot Device</button></form>
</body>
</html>
)raw";

#endif // WEBPORTALHTML_H