#include "fmtuner.hpp"

FMTuner::FMTuner()
{
    Serial.println("FMTuner setup ...");

    Preferences _prefs;
    if(!_prefs.begin("esp32radio", false))
    {
        Serial.println("unable to open preferences");
    }
    else
    {
        Serial.println("Loading presets from SPIIFFS");
        station_presets[0] = _prefs.getUShort("FMPRESET_0", 9870);
        station_presets[1] = _prefs.getUShort("FMPRESET_1", 10390);
        station_presets[2] = _prefs.getUShort("FMPRESET_2", 0);
        station_presets[3] = _prefs.getUShort("FMPRESET_3", 0);
        station_presets[4] = _prefs.getUShort("FMPRESET_4", 0);
        station_presets[5] = _prefs.getUShort("FMPRESET_5", 0);
        station_presets[6] = _prefs.getUShort("FMPRESET_6", 0);
        station_presets[7] = _prefs.getUShort("FMPRESET_7", 0);
        current_station_preset = _prefs.getUShort("LASTPRESET", 0);
        _prefs.end();

        channel = station_presets[current_station_preset];
    }

    Serial.println("Start Si4703 ...");
    Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);

    radio = new SI470X();
    radio->setup(RESET_PIN, ESP32_I2C_SDA);
    radio->setMono(false);
    radio->setMute(true);
    radio->setRds(true);
    radio->setFmDeemphasis(1);
    radio->setRdsMode(1);
    radio->setSeekThreshold(35);

    Serial.println("Chip version: " + String(radio->getChipVersion()));
    Serial.println("Device ID: " + String(radio->getDeviceId()));
    Serial.println("Firmware version: " + String(radio->getFirmwareVersion()));
    Serial.println("Manufacturer: " + String(radio->getManufacturerId()));
    
}

void FMTuner::Start()
{
    Serial.println("FMTuner start ...");

    // Initialize the Radio
    
    radio->setFrequency(channel);
    radio->setVolume(volume);
    radio->setMute(false);
}

void FMTuner::Stop()
{
    Serial.println("FMTuner stop ...");
    radio->setMute(true);
}

void FMTuner::DisplayInfo()
{
    Serial.println("Channel: " + String(channel));
    Serial.println("Volume: " + String(volume));
    Serial.println("RSSI: " + String(radio->getRssi()) + " db");
    if(_savemode)
        Serial.println("Preset save mode is: on");
    else
        Serial.println("Preset save mode is: off");
}

void FMTuner::SetSaveMode(bool onoff)
{
    _savemode = onoff;
}

void FMTuner::SwitchPreset(int num)
{
    Serial.println("FMTuner::SwitchPreset to " + String(num));
    current_station_preset = num;
    channel = station_presets[num];
    radio->setFrequency(channel);
}

void FMTuner::SaveCurrentChannel(int preset)
{
    Serial.println("Saving frequency " + String(channel)+ " as preset " + preset);
    station_presets[preset] = channel;

    SavePresets();
}

void FMTuner::SavePresets()
{
    Preferences _prefs;
    _prefs.begin("esp32radio_fmtuner", false); 
    _prefs.putUShort("FMPRESET_0", station_presets[0]);
    _prefs.putUShort("FMPRESET_1", station_presets[1]);
    _prefs.putUShort("FMPRESET_2", station_presets[2]);
    _prefs.putUShort("FMPRESET_3", station_presets[3]);
    _prefs.putUShort("FMPRESET_4", station_presets[4]);
    _prefs.putUShort("FMPRESET_5", station_presets[5]);
    _prefs.putUShort("FMPRESET_6", station_presets[6]);
    _prefs.putUShort("FMPRESET_7", station_presets[7]);
    _prefs.putUShort("LASTPRESET", current_station_preset);
    _prefs.end();
}

void FMTuner::Loop(char ch)
{
    bool channelchanged = false;
    long now = millis();

    if(_seekmode != '0' && (now - _seektimer) > 2000)
    {
        Serial.println("Seek still going on ....");
        ch = _seekmode;
    }

    switch(ch)
    {
        case 'o':
            radio->setFrequencyUp();
            channelchanged = true;
            break;
        case 'i':
            radio->setFrequencyDown();
            channelchanged = true;
            break;
        case 'O':
            radio->setFrequency(channel + 25);          
            channelchanged = true;
            break;
        case 'I':
            radio->setFrequency(channel - 25);
            channelchanged = true;
            break;
        case 'u':
            _seekmode = ch;
            radio->seek(0, 1);
            _seektimer = millis();
            channelchanged = true;
            break;
        case 'z':
            _seekmode = ch;
            radio->seek(0, 0);
            _seektimer = millis();
            channelchanged = true;
            break;
        case 't':
            _seekmode = '0';
            Serial.println("Seek stopped");
            DisplayInfo();
            break;
        case 'x':
            SetSaveMode(!_savemode);
            DisplayInfo();
            break;
        case '+':
            volume ++;
            if (volume == 16) volume = 15;
            radio->setVolume(volume);
            DisplayInfo();
            break;
        case '-':
            volume --;
            if (volume < 0) volume = 0;
            radio->setVolume(volume);
            DisplayInfo();
            break;
    }

    if (ch == 'r')
    {
        if(radio->getRdsReady())
        {    
            
            char *block0A = radio->getRdsText0A();
            char *block2A = radio->getRdsText2A();
            char *block2B = radio->getRdsText2B();
            char *time = radio->getRdsTime();
            char *text = radio->getRdsText();

            if(radio->getRdsSync())
                Serial.println("RDS is synchronized!");


            if(time != NULL)
                Serial.println("RDS Time:" +  String(time));

            if(text != NULL)
                Serial.println("RDS Text:" +  String(text));

            if(block0A != NULL)
                Serial.println("RDS 0A:" +  String(block0A));

            if(block2A != NULL)
                Serial.println("RDS 2A:" +  String(block2A));

            if(block2B != NULL)
                Serial.println("RDS 2B:" +  String(block2B));
        }
        else
            Serial.println("RDS not ready yet ....");
    }
    

    if(channelchanged)
    {
        channel = radio->getFrequency();
        if(_savemode)
            SaveCurrentChannel(current_station_preset);
        DisplayInfo();
    }
}