#include "frequency_display.hpp"

FrequencyDisplay::FrequencyDisplay()
{
    Serial.println("Setup frequency display ...");

    _u8g2 = new U8G2_MAX7219_32X8_F_4W_SW_SPI(U8G2_R0, /* clock=*/ 14, /* data=*/ 13, /* cs=*/ 15, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ U8X8_PIN_NONE);

    _u8g2->begin();
    _u8g2->clearBuffer();					// clear the internal memory
    SetFont(0);
    _u8g2->drawStr(_xoffset,_yoffset,"1234567890");
    _u8g2->sendBuffer();

    _lastUpdate = millis();
}

void FrequencyDisplay::SetFont(int fontindx)
{
    Serial.println("Freq. display set font " + String(fontindx));

    if(fontindx == _currentFont)
        return;

    switch(fontindx)
    {
        case 1:
            _u8g2->setFont(u8g2_font_5x7_tf);
            _max_chars = 6;
            _fontwidth = 5;
            break;
        default:
            _u8g2->setFont(u8g2_font_spleen6x12_me);
            _max_chars = 5;
            _fontwidth = 6;
            break;
    }   

    _currentFont = fontindx;    
    
}

void FrequencyDisplay::DisplayText(String text, int font)
{
    if(text == _displayText_original)
        return;

    Serial.println("Displaytext: " + text);

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
    //Serial.println("Updating display .... x:" + String(_start_x) + " t:" + _displayText);
    _u8g2->clearBuffer();					// clear the internal memory
    _u8g2->drawStr(_xoffset - _start_x, _yoffset,_displayText.c_str());
    _u8g2->sendBuffer();
}

void FrequencyDisplay::Loop()
{
    unsigned long now = millis();
    if((now - _lastUpdate) < 200)
        return;
    _lastUpdate = now;

    if(_displayText.length() > _max_chars)
    {
        if((now - _lastMarqueeUpdate) > (_marquee_update_interval - _displayText_original.length() * 10))
        {
            _start_x ++;
            if(_start_x > ((_displayText_original.length() + 1) * _fontwidth))
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



