#include "clock_display.hpp"

ClockDisplay::ClockDisplay()
{
    Serial.println("Initialize clock display");
    _lcd = new LiquidCrystal_I2C(0x27,16,2);

    _lcd->init();                      // initialize the lcd 

    // Print a message to the LCD.
    _lcd->backlight();
 
}

void ClockDisplay::DisplayText(String text, uint8_t row)
{
    if(text == _texts[row])
        return;
    _texts[row] = text;

    //Serial.println("Clock display: " + text);
    _lcd->clear();
    _lcd->setCursor(0,0);
    _lcd->print(_texts[0]);
    _lcd->setCursor(0,1);
    _lcd->print(_texts[1]);
    delay(100);
    
}

void ClockDisplay::Loop()
{

}