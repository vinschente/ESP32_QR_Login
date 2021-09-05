#include <Arduino.h>

#include <WiFi.h>

#define LILYGO_T5_V213
#include <boards.h>
#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <Fonts/FreeSans9pt7b.h>

#include "display.h"

#include <qrcode.h>

#include "config.h"

static void displayText_ROI(const char *str, int16_t y, uint8_t align, uint16_t roiX0, uint16_t roiX1);


String apSSID = "TTGO-T5";
String apPassword = "00000000";
IPAddress myIP;

GxIO_Class io(SPI, EPD_CS, EPD_DC, EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

/****************************************************
  ____  _____  
 / __ \|  __ \ 
| |  | | |__) |
| |  | |  _  / 
| |__| | | \ \ 
 \___\_\_|  \_\
****************************************************/
typedef enum
{
    WIFI_NONE,
    WIFI_WEP,
    WIFI_WPA,
    WIFI_WPA2,
} WIFI_Security_t;

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t color)
{
    for (int16_t i = 0; i < w; i++)
    {
        for (int16_t j = 0; j < h; j++)
        {
            display.drawPixel(x + i, y + j, color);
        }
    }
}

void drawQR(int16_t x, int16_t y, int16_t max_rect_size, String raw)
{

    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, 3, 0, raw.c_str());

    int8_t scale = max_rect_size / qrcode.size;

    for (uint8_t i = 0; i < qrcode.size; i++)
    {
        for (uint8_t j = 0; j < qrcode.size; j++)
        {
            if (qrcode_getModule(&qrcode, i, j))
            {
                drawRect(i * scale + x, j * scale + y, scale, scale, GxEPD_BLACK);
            }
        }
    }
}

String QR_Code_raw_Wifi(String ssid, String password, WIFI_Security_t security, bool hidden)
{
    String raw;

    raw = "WIFI:S:" + ssid + ";";
    if (password.length())
    {
        raw += "P:" + password + ";";
    }

    if (security == WIFI_WEP)
    {
        raw += "T:WEP;";
    }
    else if (security == WIFI_WPA || security == WIFI_WPA2)
    {
        raw += "T:WPA;";
    }

    if (hidden)
    {
        raw += "H:true;";
    }
    raw += ";";

    return raw;
}

String QR_Code_raw_URL(String url)
{
    // No change on URL nesseccary
    return url;
}

/****************************************************
 _____  _           _             
|  __ \(_)         | |            
| |  | |_ ___ _ __ | | __ _ _   _ 
| |  | | / __| '_ \| |/ _` | | | |
| |__| | \__ \ |_) | | (_| | |_| |
|_____/|_|___/ .__/|_|\__,_|\__, |
             | |             __/ |
             |_|            |___/ 
****************************************************/
enum {
    GxEPD_ALIGN_RIGHT,
    GxEPD_ALIGN_LEFT,
    GxEPD_ALIGN_CENTER,
};

static void displayText_ROI(const char *str, int16_t y, uint8_t align, uint16_t roiX0, uint16_t roiX1)
{
    int16_t x = 0;
    int16_t x1 = 0, y1 = 0;
    uint16_t w = 0, h = 0;
    display.setCursor(x, y);
    display.getTextBounds(str, x, y, &x1, &y1, &w, &h);
    switch (align) {
    case GxEPD_ALIGN_RIGHT:
        display.setCursor(roiX1 - w, y);
        break;
    case GxEPD_ALIGN_LEFT:
        display.setCursor(0+roiX0, y);
        break;
    case GxEPD_ALIGN_CENTER:
        display.setCursor(roiX0 + (roiX1-roiX0) / 2 - ((w ) / 2), y);
        break;
    default:
        break;
    }
    display.println(str);   
}

void showWifiLoginQR(void)
{
    String qrCode_raw = QR_Code_raw_Wifi(apSSID, apPassword, WIFI_WPA2, false);

    int8_t disp_size = (display.width() < display.height()) ? display.width() : display.height();

    display.fillScreen(GxEPD_WHITE);

    drawQR(0, 0, disp_size, qrCode_raw);

    displayText_ROI("Scan me", 30, GxEPD_ALIGN_CENTER, 130, 250);
    displayText_ROI("or", 50, GxEPD_ALIGN_CENTER, 130, 250);
    displayText_ROI("connect to", 70, GxEPD_ALIGN_CENTER, 130, 250);
    displayText_ROI(apSSID.c_str(), 90, GxEPD_ALIGN_CENTER, 130, 250);
    displayText_ROI(apPassword.c_str(), 110, GxEPD_ALIGN_CENTER, 130, 250);

    display.update();
}

void showWifiConfigurationUrlQR(void)
{
    String url = "http://" + myIP.toString();
    String qrCode_raw = QR_Code_raw_URL(url);

    int8_t disp_size = (display.width() < display.height()) ? display.width() : display.height();

    display.fillScreen(GxEPD_WHITE);

    drawQR(130, 0, disp_size / 2, qrCode_raw);

    displayText_ROI("Scan me", 30, GxEPD_ALIGN_CENTER, 0, 130);
    displayText_ROI("or", 50, GxEPD_ALIGN_CENTER, 0, 130);
    displayText_ROI("open URL", 70, GxEPD_ALIGN_CENTER, 0, 130);
    displayText_ROI(url.c_str(), 90, GxEPD_ALIGN_LEFT, 0, 130);

    display.update();
}

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
        showWifiLoginQR();
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        Serial.println("Station connected to ESP32 soft AP");
        connected_stations++;
        showWifiConfigurationUrlQR();
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        Serial.println("Station disconnected from ESP32 soft AP");
        connected_stations--;
        if (connected_stations == 0)
        {
            showWifiLoginQR();
        }
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

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init();
    display.setRotation(1);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(DEFALUT_FONT);

    display.fillScreen(GxEPD_WHITE);


    apPassword = String(rand() % 100000000);
    setupWiFi();
}

void loop()
{
}