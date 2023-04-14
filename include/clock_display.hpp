#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>

class ClockDisplay
{
    private:
        LiquidCrystal_I2C *_lcd;
        String _texts[2];

    public:
        ClockDisplay();
        void DisplayText(String text, uint8_t row);
        void Loop();

};