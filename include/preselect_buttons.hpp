#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "PCF8574.h"
#include "pmCommonLib.hpp"

class PreselectButtons : i2cdevice
{
    private:
        PCF8574 *_pcf8574;
        unsigned long _lastRead = 0;
        bool _active = false;
    public:
        PreselectButtons(TwoWire* twowire, uint8_t adr);
        int Loop();
};