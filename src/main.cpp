#include <Arduino.h>

#include <WiFi.h>

String apSSID = "TTGO-T5";
String apPassword = "00000000";
IPAddress myIP;

/****************************************************
__          ___ ______ _ 
\ \        / (_)  ____(_)
 \ \  /\  / / _| |__   _ 
  \ \/  \/ / | |  __| | |
   \  /\  /  | | |    | |
    \/  \/   |_|_|    |_|
****************************************************/

volatile uint8_t connected_stations = 0;

void OnWiFiEvent(WiFiEvent_t event)
{
    switch (event)
    {

    case SYSTEM_EVENT_STA_CONNECTED:
        Serial.println("ESP32 Connected to WiFi Network");
        break;
    case SYSTEM_EVENT_AP_START:
        Serial.println("ESP32 soft AP started");
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        Serial.println("Station connected to ESP32 soft AP");
        connected_stations++;
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        Serial.println("Station disconnected from ESP32 soft AP");
        connected_stations--;
        break;
    default:
        break;
    }
}

void createAccesspoint(void)
{
    Serial.println(F("Setting up AP"));
    WiFi.mode(WIFI_AP);
    Serial.println("SSID: " + apSSID);
    Serial.println("Password: " + apPassword);
    WiFi.softAP(apSSID.c_str(), apPassword.c_str());
    myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
}

void setupWiFi(void)
{
    WiFi.onEvent(OnWiFiEvent);

    createAccesspoint();
}

/****************************************************
 __  __       _       
|  \/  |     (_)      
| \  / | __ _ _ _ __  
| |\/| |/ _` | | '_ \ 
| |  | | (_| | | | | |
|_|  |_|\__,_|_|_| |_|
****************************************************/
void setup()
{
    Serial.begin(115200);

    Serial.println("ESP32 QR Code Login");

    apPassword = String(rand() % 100000000);
    setupWiFi();
}

void loop()
{
}