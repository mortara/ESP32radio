#include <Arduino.h>
#include <U8g2lib.h>

#include <SPI.h>
#include "webserial.hpp"

class FrequencyDisplay
{
    private:
        U8G2_MAX7219_32X8_F_4W_SW_SPI *_u8g2;
        String _displayText;
        String _displayText_original;
        unsigned int _max_chars = 6;
        unsigned long _marquee_update_interval = 300;
        unsigned long _lastUpdate;
        unsigned long _lastMarqueeUpdate;
        unsigned int _start_x = 0;
        bool _update = false;
        bool _active = false;
        typedef struct
        {  
            uint8_t _xoffset;
            uint8_t _yoffset;
            uint8_t _fontwidth;
            uint8_t _maxchars;
            const uint8_t *_font; 
        } FontSetting;

        FontSetting _fontsettings[2] = {
            {1, 8, 6, 6, (const uint8_t *)u8g2_font_spleen6x12_me},
            //{1, 7, 5, 7, (const uint8_t *)u8g2_font_5x7_tf},
            {1, 7, 5, 9, (const uint8_t *)u8g2_font_profont10_tn        }, 
        };

        FontSetting _currentFont;
        uint8_t _currenFontIndex = -1;
        void updateScreen();
    public:
        FrequencyDisplay();
        void SetFont(uint8_t fontindx);
        void DisplayText(String text, uint8_t font);
        void TurnOnOff(bool on);
        void Loop();
};