#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "PCF8574.h"

class PreselectLeds
{
    private:
        PCF8574 *_pcf8754;
        
    public:
        PreselectLeds(TwoWire *twowire, uint8_t adr);
        void SetLed(uint8_t num);
        void Loop();
};