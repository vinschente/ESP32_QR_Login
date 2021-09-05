#include <Arduino.h>

void setup()
{
    Serial.begin(115200);

    Serial.println("ESP32 QR Code Login");
}

void loop()
{
    static int count = 0;

    delay(1000);
    Serial.println(count);
    count++;
}