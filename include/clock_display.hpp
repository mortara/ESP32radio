#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>

class ClockDisplay
{
    private:
        LiquidCrystal_I2C *_lcd;
    public:
        ClockDisplay();
        void DisplayText(String text);
        void Loop();

};