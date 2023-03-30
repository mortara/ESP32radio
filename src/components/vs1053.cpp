#include "vs1053.hpp"


VS1053Player::VS1053Player()
{
    Serial.println("Creating vs1053 object");
    _player = new Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, -1);
    _player->begin();
    _player->setVolume(100, 100);
    //_player->softReset();
}

void VS1053Player::Begin()
{
    Serial.println("starting vs1053");
    SetVolume(_volume);
    //_player->sineTest(0x44, 500);
}

void VS1053Player::End()
{
    Serial.println("stopping vs1053");
    _player->stopPlaying();
    _player->setVolume(100, 100);
}

void VS1053Player::SetVolume(uint8_t vol)
{
    _volume = vol;
    Serial.println("set volume on vs1053 to " + String(vol));
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