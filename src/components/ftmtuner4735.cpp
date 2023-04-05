#include "fmtuner4735.hpp"

#define AM_FUNCTION 1
#define FM_FUNCTION 0

#define FM_BAND_TYPE 0
#define MW_BAND_TYPE 1
#define SW_BAND_TYPE 2
#define LW_BAND_TYPE 3

FMTuner4735::FMTuner4735()
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

    Serial.println("Start Si4735 ...");
    digitalWrite(RESET_PIN, HIGH);
+
    Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);
    //digitalWrite(RESET_PIN, HIGH);
    radio = new SI4735();
    
    radio->getDeviceI2CAddress(RESET_PIN);
    radio->setMaxDelayPowerUp(500);
    radio->setup(RESET_PIN,0, FM_FUNCTION, SI473X_ANALOG_AUDIO, 1, 0);
    radio->setSeekFmSpacing(10);
    radio->setSeekFmLimits(8750, 10800);
    radio->setSeekAmRssiThreshold(50);
    radio->setSeekAmSrnThreshold(20);
    radio->setSeekFmRssiThreshold(5);
    radio->setSeekFmSrnThreshold(5);

    radio->setTuneFrequencyAntennaCapacitor(0);
    delay(100);
    radio->setFM(8400, 10800, 10390, 10);
    radio->setFMDeEmphasis(2);

    radio->RdsInit();
    radio->setRdsConfig(1, 2, 2, 2, 2);


    delay(100);
    
    Serial.println("Firmware version: " + String(radio->getFirmwareCHIPREV()));
    
    
}

void FMTuner4735::Start()
{
    Serial.println("FMTuner start ...");

    // Initialize the Radio
    radio->radioPowerUp();
    radio->setFrequency(channel);
    radio->setVolume(volume);
    
    DisplayInfo();
}

void FMTuner4735::Stop()
{
    Serial.println("FMTuner stop ...");
    radio->powerDown();
}

void FMTuner4735::DisplayInfo()
{
    Serial.println("Radio info:");
    radio->getStatus();
    radio->getCurrentReceivedSignalQuality();
    Serial.print("You are tuned on ");
    if (radio->isCurrentTuneFM())
    {
        Serial.print(String(channel / 100.0, 2));
        Serial.print("MHz ");
        Serial.print((radio->getCurrentPilot()) ? "STEREO" : "MONO");
    }
    else
    {
        Serial.print(channel);
        Serial.print("kHz");
    }
    Serial.print(" [SNR:");
    Serial.print(radio->getCurrentSNR());
    Serial.print("dB");

    Serial.print(" Signal:");
    Serial.print(radio->getCurrentRSSI());
    Serial.println("dBuV]");

    Serial.println("Volume: " + String(volume));
    
    if(_savemode)
        Serial.println("Preset save mode is: on");
    else
        Serial.println("Preset save mode is: off");
}

void FMTuner4735::SetSaveMode(bool onoff)
{
    _savemode = onoff;
}

void FMTuner4735::SwitchPreset(int num)
{
    Serial.println("FMTuner::SwitchPreset to " + String(num));
    current_station_preset = num;
    channel = station_presets[num];
    radio->setFrequency(channel);
    DisplayInfo();
}

void FMTuner4735::SaveCurrentChannel(int preset)
{
    Serial.println("Saving frequency " + String(channel)+ " as preset " + preset);
    station_presets[preset] = channel;

    SavePresets();
}

void FMTuner4735::SavePresets()
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

void FMTuner4735::Loop(char ch)
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
            radio->frequencyUp();
            channelchanged = true;
            break;
        case 'i':
            radio->frequencyDown();
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
            radio->seekStationUp();
            _seektimer = millis();
            channelchanged = true;
            break;
        case 'z':
            _seekmode = ch;
            radio->seekStationDown();
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
            radio->volumeUp();
            volume = radio->getVolume();
            DisplayInfo();
            break;
        case '-':
            radio->volumeDown();
            volume = radio->getVolume();
            DisplayInfo();
            break;
    }

    if (ch == 'r')
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

    delay(100);    

    if(channelchanged)
    {
        channel = radio->getFrequency();
        radio->setVolume(volume);
        if(_savemode)
            SaveCurrentChannel(current_station_preset);
        DisplayInfo();
    }
}