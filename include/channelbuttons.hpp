#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "PCF8574.h"
#include "i2cdevice.hpp"

class ChannelButtons : i2cdevice
{
    private:
        uint8_t _address;
        PCF8574 *_pcf8754;
        TwoWire *_i2cwire;
        unsigned long _lastRead = 0;
        int currentchannel = 0;
        bool _active = false;
        int readInputs();
    public:
        ChannelButtons(TwoWire *twowire, uint8_t adr);
        int Loop();

};