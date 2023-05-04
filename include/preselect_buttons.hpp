#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "PCF8574.h"

class PreselectButtons
{
    private:
        uint8_t _address;
        PCF8574 *_pcf8754;
        TwoWire *_i2cwire;
        unsigned long _lastRead = 0;
        bool _active = false;
    public:
        PreselectButtons(TwoWire *twowire, uint8_t adr);
        int Loop();
};