#include "clock_display.hpp"

void ClockDisplayClass::StartUp(uint8_t adr)
{
    i2cdevice::Setup(&Wire, adr);

    if(isActive())
        pmLogging.LogLn("Initialize clock display");
    else
    {
        pmLogging.LogLn("clock display not found");
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

    _texts[row] = text;

    if(!_active || _lcd == nullptr)
    {
        pmLogging.LogLn(String(row) + ": " + text);
        return;
    }

    // Feste Länge, keine String-Kopien
    char buf[17];
    size_t len = text.length();
    if(len > 16) len = 16;
    text.toCharArray(buf, len + 1);
    for(size_t i = len; i < 16; ++i) buf[i] = ' ';
    buf[16] = '\0';

    _lcd->setCursor(0,row);
    _lcd->print(buf);
}

void ClockDisplayClass::Loop()
{
    if(_texts[0].length() > 16 && _active)
    {
        unsigned long _now = millis();
        if(_now - _scroll_row1_timer > 500UL)
        {
            // Erzeuge den Scroll-Text als char-Array
            String scrollText = _texts[0] + "   " + _texts[0];
            size_t scrollLen = scrollText.length();
            char buf[17];
            for(size_t i = 0; i < 16; ++i)
            {
                if(i + _scroll_row1_offset < scrollLen)
                    buf[i] = scrollText[i + _scroll_row1_offset];
                else
                    buf[i] = ' ';
            }
            buf[16] = '\0';
            _lcd->setCursor(0,0);
            _lcd->print(buf);

            _scroll_row1_timer = _now;
            _scroll_row1_offset++;
            if(_scroll_row1_offset >= scrollLen - 15)
                _scroll_row1_offset = 0;
        }
    }
}

void ClockDisplayClass::TurnOnOff(bool on)
{
    pmLogging.LogLn("turning clock display on/off: " + String(on));

    if(on)
        _lcd->backlight();
    else
        _lcd->noBacklight();

}

ClockDisplayClass ClockDisplay; 