#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "PCF8574.h"
#include "i2cdevice.hpp"

class ClockButtons : i2cdevice
{
    private:
        PCF8574 *_pcf;
        unsigned long _lastRead = 0;
        
        bool _active = false;
        int readInputs();
    public:
        ClockButtons(TwoWire &twowire, uint8_t adr);
        void DisplayInfo();
        int Loop();

};