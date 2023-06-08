#include "frequency_display.hpp"

FrequencyDisplay::FrequencyDisplay()
{
    WebSerialLogger.println("Setup frequency display ...");

    _u8g2 = new U8G2_MAX7219_32X8_F_4W_SW_SPI(U8G2_R2, /* clock=*/ 14, /* data=*/ 13, /* cs=*/ 15, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ U8X8_PIN_NONE);
    //_u8g2 = new U8G2_MAX7219_32X8_F_4W_HW_SPI(U8G2_R2,/* cs=*/ 15, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ U8X8_PIN_NONE);
    if(!_u8g2->begin())
    {
        WebSerialLogger.println("display not found!");
        return;
    }
    _active = true;
    _u8g2->setContrast(5);
    //_u8g2->setPowerSave(1);
    _u8g2->clearBuffer();					// clear the internal memory
    SetFont(0);
    _u8g2->drawStr(_currentFont._xoffset,_currentFont._yoffset,"HELLO!");
    _u8g2->sendBuffer();
    _lastUpdate = millis();
    
}

void FrequencyDisplay::SetFont(uint8_t fontindx)
{
    if(!_active)
        return;
    
    if(_currenFontIndex == fontindx)
        return;

    WebSerialLogger.println("Freq. display set font " + String(fontindx));

    _currentFont = _fontsettings[fontindx];
    _u8g2->setFont(_currentFont._font);
    _currenFontIndex = fontindx;
}

void FrequencyDisplay::DisplayText(String text, uint8_t font)
{
    if(!_active)
        return;

    if(text == _displayText_original)
        return;

    //WebSerialLogger.println("Displaytext: " + text);

    SetFont(font);

    _displayText_original = text;

    if(text.length() > _max_chars)
    {
        _displayText = text + " " + text;
    }
    else
        _displayText = text;  

    _update = true;
    _start_x = 0;
}

void FrequencyDisplay::updateScreen()
{
    if(!_active)
        return;
    //Serial.println("Updating display .... x:" + String(_start_x) + " t:" + _displayText);
    _u8g2->clearBuffer();					// clear the internal memory
    _u8g2->drawStr(_currentFont._xoffset - _start_x, _currentFont._yoffset,_displayText.c_str());
    _u8g2->sendBuffer();
}

void FrequencyDisplay::Loop()
{
    if(!_active)
        return;

    unsigned long now = millis();
    if((now - _lastUpdate) < 300)
        return;
    _lastUpdate = now;

    if(_displayText.length() > _max_chars)
    {
        if((now - _lastMarqueeUpdate) > (_marquee_update_interval - _displayText_original.length() * 10))
        {
            _start_x ++;
            if(_start_x > ((_displayText_original.length() + 1) * _currentFont._fontwidth))
                _start_x = 0;

            _lastMarqueeUpdate = now;
            _update = true;
        }
    }
    else
        _start_x = 0;

    if(_update)
    {
        updateScreen();
        _update = false;
    }
}