#include <Arduino.h>
#include <Adafruit_INA219.h>

#include "i2cdevice.hpp"

class PowerSensor : i2cdevice
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
        PowerSensor(uint8_t adr);
        void Loop();
        void DisplayInfo();
};