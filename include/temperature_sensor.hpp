#include <Arduino.h>
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include "mqtt.hpp"
#include "i2cdevice.hpp"

class TemperatureSensor : i2cdevice
{
    private:
        Adafruit_BMP085 *_bmp = NULL;
        unsigned long _lastRead;
        bool setupmqtt = false;

        bool mqttSetup();

    public:
        TemperatureSensor(uint8_t adr);
        void Loop();

};