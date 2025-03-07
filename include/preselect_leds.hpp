#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "PCF8574.h"
#include "pmCommonLib.hpp"

class PreselectLeds : i2cdevice
{
    private:
        PCF8574 *_pcf8754;
        bool _active = false;
        
    public:
        PreselectLeds(TwoWire *twowire, uint8_t adr);
        void SetLed(uint8_t num);
        void ShowPercentage(int value, int max);
};