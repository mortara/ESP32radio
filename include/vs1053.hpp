#include <Arduino.h>
#include <Adafruit_VS1053.h>

#ifndef VS1053_h
#define VS1053_h

#define CLK 18       // SPI Clock, shared with SD card
#define MISO 19      // Input data, from VS1053/SD card
#define MOSI 23      // Output data, to VS1053/SD card

#define BREAKOUT_RESET  -1     // VS1053 reset pin (output)
#define BREAKOUT_XDCS    16    // VS1053 Data/command select pin (output)
#define BREAKOUT_SD_CS    17
#define BREAKOUT_MP3_CS    5

// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define BREAKOUT_DREQ 4       // VS1053 Data request, ideally an Interrupt pin

class VS1053Player
{
    private:
        Adafruit_VS1053_FilePlayer  *_player;
        uint8_t _volume = 10;

    public:
        VS1053Player();

        void Begin();
        void End();
        void SetVolume(uint8_t vol);
        void PlayData(uint8_t *buffer, uint8_t size);
        bool ReadyForData();
        void ExecuteCommand(char ch);
};

#endif