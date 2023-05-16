#include "vs1053.hpp"


VS1053Player::VS1053Player()
{
    WebSerialLogger.println("Creating vs1053 object");
    _player = new Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_MP3_CS, BREAKOUT_XDCS, BREAKOUT_DREQ, BREAKOUT_SD_CS);
    if (! _player->begin()) { // initialise the music player
        WebSerialLogger.println("Couldn't find VS1053, do you have the right pins defined?");
    }
    _player->setVolume(100, 100);
    //_player->softReset();

    if (!SD.begin(BREAKOUT_SD_CS)) {
        WebSerialLogger.println("SD failed, or not present");
    }
    else
        WebSerialLogger.println("SD OK!");

}

void VS1053Player::Begin()
{
    WebSerialLogger.println("starting vs1053");
    SetVolume(_volume);
    //_player->sineTest(0x44, 500);
}

void VS1053Player::End()
{
    WebSerialLogger.println("stopping vs1053");
    _player->stopPlaying();
    _player->setVolume(100, 100);
}

void VS1053Player::SetVolume(uint8_t vol)
{
    _volume = vol;
    WebSerialLogger.println("set volume on vs1053 to " + String(vol));
    _player->setVolume(vol, vol);
}

void VS1053Player::PlayData(uint8_t *buffer, uint8_t size)
{
    //Serial.println("playing data on vs1053");
    _player->playData(buffer, size);
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
        _player->sineTest(0x44, 500);
    }
}