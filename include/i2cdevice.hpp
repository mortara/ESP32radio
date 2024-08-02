#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "webserial.hpp"

#ifndef I2CDEVICE_H
#define I2CDEVICE_H

class i2cdevice
{
    private:
        bool _active = false;

    public:
        i2cdevice()
        {

        }

        i2cdevice(uint8_t address)
        {
            Setup(&Wire, address);
        }


        i2cdevice(TwoWire *wire, uint8_t address)
        {
            Setup(wire, address);
        }

        void Setup(TwoWire *wire, uint8_t address)
        {
            char hexadecimalnum [3];
            sprintf(hexadecimalnum, "%X", address);

            int wirenum = 1;
            if(wire == &Wire)
                wirenum = 0;

            WebSerialLogger.println("Testing device 0x" + String(hexadecimalnum) + " on wire " + String(wirenum));
            wire->beginTransmission(address);
            uint8_t error = wire->endTransmission();
            if (error != 0)
            {
                WebSerialLogger.println("Device 0x" + String(hexadecimalnum) + " not found on wire 2");
                return;
            }
            _active = true;
        }

        bool isActive()
        {
            return _active;
        }
};

#endif