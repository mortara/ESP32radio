#include <Arduino.h>
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include "pmCommonLib.hpp"


#ifndef TEMPSENSOR_H
#define TEMPSENSOR_H
class TemperatureSensorClass : i2cdevice
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
        void Begin(uint8_t adr);
        void Loop();
        void DisplayInfo();
        float GetLastTemperatureReading();
        
};

extern TemperatureSensorClass TemperatureSensor1;

#endif