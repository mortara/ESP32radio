#include <Arduino.h>
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include "mqtt.hpp"
#include "i2cdevice.hpp"

class TemperatureSensor : i2cdevice
{
    private:
        Adafruit_BMP085 _bmp;
        unsigned long _lastRead;
        bool setupmqtt = false;
        bool _active = false;
        float _temperature = 0;
        int32_t _pressure = 0;
        float _altitude = 0;
        bool mqttSetup();

    public:
        TemperatureSensor(uint8_t adr);
        void Loop();
        void DisplayInfo();
        
};