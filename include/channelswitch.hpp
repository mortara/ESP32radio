#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "PCF8574.h"

class ChannelSwitch
{
    private:
        uint8_t _address;
        uint8_t _channel;
        bool _running;
        PCF8574 *_relais;
        TwoWire *_i2cwire;
        const uint8_t offval = HIGH;
        const uint8_t onval = LOW;
        bool _active = false;
    public:
        ChannelSwitch(TwoWire *twowire, uint8_t adr_relais);

        void SetChannel(uint8_t i);
        void TurnAllOff();
};