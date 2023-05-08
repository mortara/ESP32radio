#include "clock_display.hpp"

ClockDisplay::ClockDisplay() : i2cdevice(0x27)
{
    if(isActive())
        WebSerialLogger.println("Initialize clock display");
    else
    {
        WebSerialLogger.println("clock display not found");
    }

    _lcd = new LiquidCrystal_I2C(0x27,16,2);
    _lcd->init();                      // initialize the lcd 
    // Print a message to the LCD.
    _lcd->backlight();
 
    _active = true;
}

void ClockDisplay::DisplayText(String text, uint8_t row)
{
    if(text == _texts[row])
        return;
    _texts[row] = text;
    
    if(!_active)
    {
        WebSerialLogger.println(String(row) + ": " + text);
        return;
    }

    //Serial.println("Clock display: " + text);
    //_lcd->clear();

    String t1 = text;
    while(t1.length() < 16)
        t1 = t1 + " ";

    if(t1.length() <= 16)
    {
        _lcd->setCursor(0,row);
        _lcd->print(t1);
    }
}

void ClockDisplay::Loop()
{
    if(!_active)
        return;

    unsigned long _now = millis();

    if(_texts[0].length() > 16)
    {
        if(_now - _scroll_row1_timer > 500)
        {
            String t1 = _texts[0] + "   " + _texts[0];
            String t2 = t1.substring(_scroll_row1_offset, _scroll_row1_offset + 16);
            _lcd->setCursor(0,0);
            _lcd->print(t2);

            _scroll_row1_timer = _now;
            _scroll_row1_offset++;
            if(_scroll_row1_offset == 17)
                _scroll_row1_offset = 0;
        }
    }
}