#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "PCF8574.h"
#include "i2cdevice.hpp"

class PreselectLeds : i2cdevice
{
    private:
        PCF8574 *_pcf8754;
        bool _active = false;
        
    public:
        PreselectLeds(TwoWire *twowire, uint8_t adr);
        void SetLed(uint8_t num);
        void Loop();
};