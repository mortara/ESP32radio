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
    //_lcd->clear();

    String t1 = _texts[0];
    while(t1.length() < 16)
        t1 = t1 + " ";

    String t2 = _texts[1];
    while(t2.length() < 16)
        t2 = t2 + " ";

    _lcd->setCursor(0,0);
    _lcd->print(t1);
    _lcd->setCursor(0,1);
    _lcd->print(t2);
    
}

void ClockDisplay::Loop()
{

}