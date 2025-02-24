#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "PCF8574.h"
#include "I2C/i2cdevice.hpp"
#include "Webserial/webserial.hpp"

class ChannelButtons : i2cdevice
{
    private:
        PCF8574 *_pcf8754;

        unsigned long _lastRead = 0;
        int currentchannel = 0;
        bool _active = false;
    public:
        ChannelButtons(TwoWire* twowire, uint8_t adr);
        int Loop();

};