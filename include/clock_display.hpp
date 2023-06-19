#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include "i2cdevice.hpp"

class ClockDisplay : i2cdevice
{
    private:
        LiquidCrystal_I2C *_lcd;
        String _texts[2];

        unsigned long _scroll_row1_timer;
        uint8_t _scroll_row1_offset;
        bool _active = false;

    public:
        ClockDisplay(uint8_t adr);
        void DisplayText(String text, uint8_t row);
        void Loop();
        void TurnOnOff(bool on);

};