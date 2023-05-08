#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "PCF8575.h"
#include "i2cdevice.hpp"

class ClockButtons : i2cdevice
{
    private:
        uint8_t _address;
        PCF8575 *_pcf8755;
        TwoWire *_i2cwire;
        unsigned long _lastRead = 0;
        
        bool _active = false;
        int readInputs();
    public:
        ClockButtons(TwoWire *twowire, uint8_t adr);
        int Loop();

};