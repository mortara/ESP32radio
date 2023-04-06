#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>

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

        uint8_t _xoffset = 1;
        uint8_t _yoffset = 8;
        uint8_t _fontwidth = 5;
        uint8_t _currentFont = -1;

        void updateScreen();
    public:
        FrequencyDisplay();
        void SetFont(int fontindx);
        void DisplayText(String text, int font);
        void Loop();
};