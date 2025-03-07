#include "vs1053.hpp"


void VS1053Player::Setup()
{
    pmLogging.LogLn("Creating vs1053 object");
    _player = new Adafruit_VS1053_FilePlayer(MOSI, MISO, CLK, BREAKOUT_RESET, BREAKOUT_MP3_CS, BREAKOUT_XDCS, BREAKOUT_DREQ, BREAKOUT_SD_CS);
    if (! _player->begin()) { // initialise the music player
        pmLogging.LogLn("Couldn't find VS1053, do you have the right pins defined?");
    }
    else
        _active = true;
    
    _player->setVolume(100, 100);
    //_player->softReset();
/*
    if (!SD.begin(BREAKOUT_SD_CS)) {
        pmLogging.LogLn("SD failed, or not present");
    }
    else
        pmLogging.LogLn("SD OK!");
*/
}

void VS1053Player::Begin()
{
    pmLogging.LogLn("starting vs1053");
    SetVolume(_volume);
    //_player->sineTest(0x44, 500);
}

void VS1053Player::End()
{
    pmLogging.LogLn("stopping vs1053");
    //_player->stopPlaying();
    _player->setVolume(100, 100);
}

void VS1053Player::DisplayInfo()
{
    pmLogging.LogLn("VS1053:");
    pmLogging.LogLn("Active = " + String(_active));
    pmLogging.LogLn("Mode = " + String(_player->sciRead(VS1053_REG_MODE)));
    pmLogging.LogLn("Stat = " + String(_player->sciRead(VS1053_REG_STATUS)));
    pmLogging.LogLn("ClkF = " + String(_player->sciRead(VS1053_REG_CLOCKF)));
    pmLogging.LogLn("Vol. = " + String(_player->sciRead(VS1053_REG_VOLUME)));
    
}

void VS1053Player::SetVolume(uint8_t vol)
{
    _volume = vol;
    pmLogging.LogLn("set volume on vs1053 to " + String(vol));
    _player->setVolume(vol, vol);
}

void VS1053Player::PlayData(uint8_t size)
{
    //Serial.println("playing data on vs1053");
    _player->playData(Mp3buffer, size);
}

bool VS1053Player::ReadyForData()
{
    return _player->readyForData();
}

void VS1053Player::ExecuteCommand(char ch)
{
    if (ch == '+') 
    {
        if(_volume >= 10)
            SetVolume(_volume - 10);
    } else if (ch == '-') 
    {
        if(_volume<= 90)
            SetVolume(_volume + 10);
    } else if (ch == 't') 
    {
        pmLogging.LogLn("Beep!");
        _player->sineTest(0x44, 500);
    }
}

VS1053Player MP3Player;