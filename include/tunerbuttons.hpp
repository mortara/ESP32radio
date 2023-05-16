#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "PCF8574.h"
#include "i2cdevice.hpp"

class TunerButtons : i2cdevice
{
    private:
        PCF8574 *_pcf8754;
        unsigned long _lastRead = 0;
        
    public:
        TunerButtons(TwoWire &twowire, uint8_t adr);
        int Loop();
        bool SavePresetButtonPressed = false;
};