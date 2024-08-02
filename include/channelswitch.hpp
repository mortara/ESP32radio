#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "PCF8574.h"
#include "i2cdevice.hpp"

class ChannelSwitch : i2cdevice
{
    private:

        bool _running;
        PCF8574 *_relais;

        const uint8_t offval = HIGH;
        const uint8_t onval = LOW;
        bool _active = false;
    public:
        ChannelSwitch(TwoWire *twowire, uint8_t adr_relais);

        void SetChannel(uint8_t i);
        void TurnAllOff();
};