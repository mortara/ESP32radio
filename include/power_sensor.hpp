#include <Arduino.h>
#include <Adafruit_INA219.h>

#include "i2cdevice.hpp"

class PowerSensor : i2cdevice
{
    private:
        Adafruit_INA219 *_ina = NULL;
        unsigned long _lastRead;
        bool setupmqtt = false;

        bool mqttSetup();

    public:
        PowerSensor(uint8_t adr);
        void Loop(char ch);
};