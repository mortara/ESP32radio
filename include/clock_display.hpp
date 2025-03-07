#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <SPI.h>
#include "pmCommonLib.hpp"

#ifndef CLOCKDISPLAY_H
#define CLOCKDISPLAY_H

class ClockDisplayClass : i2cdevice
{
    private:
        hd44780_I2Cexp *_lcd = nullptr;
        String _texts[2];

        unsigned long _scroll_row1_timer;
        uint8_t _scroll_row1_offset;
        bool _active = false;
        uint8_t i2cadr;
    public:

        void StartUp(uint8_t adr);
        void DisplayText(String text, uint8_t row);
        void Loop();
        void TurnOnOff(bool on);

};

extern ClockDisplayClass ClockDisplay;

#endif