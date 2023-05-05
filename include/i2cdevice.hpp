#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#ifndef I2CDEVICE_H
#define I2CDEVICE_H

class i2cdevice
{
    private:
        uint8_t _address;
        TwoWire *_i2cwire;
        bool _active = false;

    public:
        i2cdevice(uint8_t address)
        {
            Serial.printf("Testing device 0x%02X \r\n", address);
            Wire.beginTransmission(address);
            uint8_t error = Wire.endTransmission();
            if (error != 0)
            {
                Serial.printf("Device 0x%02X not found! \r\n", address);
                return;
            }
            _active = true;
        }


        i2cdevice(TwoWire *wire, uint8_t address)
        {
            Serial.printf("Testing device 0x%02X on wire 2 \r\n", address);
            wire->beginTransmission(address);
            uint8_t error = wire->endTransmission();
            if (error != 0)
            {
                Serial.printf("Device 0x%02X not found on wire 2 \r\n", address);
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