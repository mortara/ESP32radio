#include <Arduino.h>
#include <Adafruit_INA219.h>

#include "i2cdevice.hpp"

#ifndef POWERSENSOR_H
#define POWERSENSOR_H

class PowerSensorClass : i2cdevice
{
    private:
        Adafruit_INA219 _ina;
        unsigned long _lastRead;
        bool setupmqtt = false;
        bool _active = false;
        float _current = 0;
        float _busvoltage = 0;
        float _shuntvoltage = 0;
        float _loadvoltage = 0;
        float _power = 0;
        bool mqttSetup();

    public:
        void Begin(uint8_t adr);
        void Loop();
        void DisplayInfo();

        float GetLastCurrentReading();
};

extern PowerSensorClass PowerSensor;

#endif