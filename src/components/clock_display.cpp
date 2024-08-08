#include "clock_display.hpp"

void ClockDisplayClass::StartUp(uint8_t adr)
{
    i2cdevice::Setup(&Wire, adr);

    if(isActive())
        WebSerialLogger.println("Initialize clock display");
    else
    {
        WebSerialLogger.println("clock display not found");
    }

    _lcd = new hd44780_I2Cexp(adr);
    _lcd->begin(16,2);
    //_lcd->backlight();
    
    _active = true;
}

void ClockDisplayClass::DisplayText(String text, uint8_t row)
{
    if(text == _texts[row])
        return;

    _texts[row] = String(text);
    
    if(!_active)
    {
        WebSerialLogger.println(String(row) + ": " + text);
        return;
    }

    //Serial.println("Clock display: " + text);
    //_lcd->clear();

    String t1 = text;
    while(t1.length() < 16)
        t1 += " ";

    _lcd->setCursor(0,row);

    if(t1.length() <= 16)
    {
        _lcd->print(t1);
    }
    else
    {
        _lcd->print(t1.substring(0,15));
    }
}

void ClockDisplayClass::Loop()
{
    if(_texts[0].length() > 16 && _active)
    {
        unsigned long _now = millis();
        if(_now - _scroll_row1_timer > 500UL)
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

void ClockDisplayClass::TurnOnOff(bool on)
{
    WebSerialLogger.println("turning clock display on/off: " + String(on));

    if(on)
        _lcd->backlight();
    else
        _lcd->noBacklight();

}

ClockDisplayClass ClockDisplay; 