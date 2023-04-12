#include "clock_display.hpp"

ClockDisplay::ClockDisplay()
{
    Serial.println("Initialize clock display");
    _lcd = new LiquidCrystal_I2C(0x27,16,2);

    _lcd->init();                      // initialize the lcd 
    // Print a message to the LCD.
    _lcd->backlight();
 
}

void ClockDisplay::DisplayText(String text)
{
    Serial.println("Clock display: " + text);
    _lcd->setCursor(0,0);
    _lcd->print(text);
}

void ClockDisplay::Loop()
{

}