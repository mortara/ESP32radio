#include <Arduino.h>

#define PREFER_SDFAT_LIBRARY YES
#include <Adafruit_VS1053.h>
#include "Webserial/webserial.hpp"

#ifndef VS1053_h
#define VS1053_h

#define CLK 6       // SPI Clock, shared with SD card
#define MISO 7      // Input data, from VS1053/SD card
#define MOSI 18      // Output data, to VS1053/SD card

#define BREAKOUT_RESET  -1     // VS1053 reset pin (output)
#define BREAKOUT_XDCS    4    // VS1053 Data/command select pin (output)
#define BREAKOUT_SD_CS    5
#define BREAKOUT_MP3_CS    16



// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define BREAKOUT_DREQ 15       // VS1053 Data request, ideally an Interrupt pin
#define MP3buffersize 32
#define MinSizeToPlay 32
class VS1053Player
{
    private:
        Adafruit_VS1053_FilePlayer  *_player;
        uint8_t _volume = 10;
        bool _active = false;
        
    public:
        void Setup();

        void Begin();
        void End();
        void SetVolume(uint8_t vol);
        void PlayData(uint8_t size);
        bool ReadyForData();
        void ExecuteCommand(char ch);
        void DisplayInfo();

        uint8_t Mp3buffer[MP3buffersize];
};

extern VS1053Player MP3Player;

#endif