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
        _station_presets[0] = _prefs.getUShort("FMPRESET_0", 9870);
        _station_presets[1] = _prefs.getUShort("FMPRESET_1", 10390);
        _station_presets[2] = _prefs.getUShort("FMPRESET_2", 0);
        _station_presets[3] = _prefs.getUShort("FMPRESET_3", 0);
        _station_presets[4] = _prefs.getUShort("FMPRESET_4", 0);
        _station_presets[5] = _prefs.getUShort("FMPRESET_5", 0);
        _station_presets[6] = _prefs.getUShort("FMPRESET_6", 0);
        _station_presets[7] = _prefs.getUShort("FMPRESET_7", 0);
        _current_station_preset = _prefs.getUShort("LASTPRESET", 0);
        _prefs.end();

        currentFrequency = _station_presets[_current_station_preset];
    }

    Serial.println("Start Si4735 ...");
    digitalWrite(RESET_PIN, HIGH);
+
    Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);
    //digitalWrite(RESET_PIN, HIGH);
    _radio = new SI4735();
    
    _radio->getDeviceI2CAddress(RESET_PIN);
    _radio->setMaxDelayPowerUp(500);
    _radio->setup(RESET_PIN,0, FM_FUNCTION, SI473X_ANALOG_AUDIO, 1, 0);
    _radio->setSeekFmSpacing(10);
    _radio->setSeekFmLimits(8750, 10800);
    _radio->setSeekAmRssiThreshold(50);
    _radio->setSeekAmSrnThreshold(20);
    _radio->setSeekFmRssiThreshold(5);
    _radio->setSeekFmSrnThreshold(5);
    _radio->setFmSoftMuteMaxAttenuation(0);
    _radio->setAmSoftMuteMaxAttenuation(0);
    _radio->setTuneFrequencyAntennaCapacitor(0);
    delay(100);
    _radio->setFM(8400, 10800, 10390, 10);
    _radio->setFMDeEmphasis(2);

    _radio->RdsInit();
    _radio->setRdsConfig(1, 2, 2, 2, 2);


    delay(100);
    
    Serial.println("Firmware version: " + String(_radio->getFirmwareCHIPREV()));
    
    
}

void FMTuner4735::Start()
{
    Serial.println("FMTuner start ...");

    // Initialize the Radio
    _radio->radioPowerUp();
    _radio->setVolume(_volume);
    
    DisplayInfo();
}

void FMTuner4735::useBand(int bandIdx)
{
  if (_bands[bandIdx].bandType == FM_BAND_TYPE)
  {
    currentMode = FM;
    _radio->setTuneFrequencyAntennaCapacitor(0);
    _radio->setFM(_bands[bandIdx].minimumFreq, _bands[bandIdx].maximumFreq, _bands[bandIdx].currentFreq, tabFmStep[_bands[bandIdx].currentStepIdx]);
    _radio->setSeekFmLimits(_bands[bandIdx].minimumFreq, _bands[bandIdx].maximumFreq);
    _radio->setRdsConfig(1, 2, 2, 2, 2);
    _radio->setFifoCount(1);
    
    bwIdxFM = _bands[bandIdx].bandwidthIdx;
    _radio->setFmBandwidth(bandwidthFM[bwIdxFM].idx);    
  }
  else
  {

    disableAgc = _bands[bandIdx].disableAgc;
    agcIdx = _bands[bandIdx].agcIdx;
    agcNdx = _bands[bandIdx].agcNdx;
    avcIdx = _bands[bandIdx].avcIdx;

    // char str[100];
    // sprintf(str,"Pos %2.2d | disableAgc %2.2d  | agcIdx %2.2d | agcNdx %2.2d | avcIdx %2.2d", bandIdx, disableAgc, agcIdx, agcNdx, avcIdx );
    // Serial.println(str);

    
    // set the tuning capacitor for SW or MW/LW
    _radio->setTuneFrequencyAntennaCapacitor((_bands[bandIdx].bandType == MW_BAND_TYPE || _bands[bandIdx].bandType == LW_BAND_TYPE) ? 0 : 1);

    currentMode = AM;
    _radio->setAM(_bands[bandIdx].minimumFreq, _bands[bandIdx].maximumFreq, _bands[bandIdx].currentFreq, tabAmStep[_bands[bandIdx].currentStepIdx]);

    bwIdxAM = _bands[bandIdx].bandwidthIdx;
    _radio->setBandwidth(bandwidthAM[bwIdxAM].idx, 1);
    _radio->setAmSoftMuteMaxAttenuation(softMuteMaxAttIdx); // Soft Mute for AM or SSB
    _radio->setAutomaticGainControl(disableAgc, agcNdx);

    
    _radio->setSeekAmLimits(_bands[bandIdx].minimumFreq, _bands[bandIdx].maximumFreq); // Consider the range all defined current band
    _radio->setSeekAmSpacing(5); // Max 10kHz for spacing
    _radio->setAvcAmMaxGain(avcIdx);
  }
  delay(100);
  currentFrequency = _bands[bandIdx].currentFreq;
  currentStepIdx = _bands[bandIdx].currentStepIdx;

}

void FMTuner4735::SwitchBand(uint8_t band)
{
    switch(band)
    {
        case 1: // LW
            useBand(0);
            break;
        case 2: // MW
            useBand(1);
            break;
        case 3: // SW1
            useBand(2);
            break;
        case 4: // SW2
            useBand(3);
            break;
        case 5: // SW3
            useBand(4);
            break;
        case 6:  // FM 
            useBand(5);
            break;
    }

    delay(100);

    band = band;
}

void FMTuner4735::Stop()
{
    Serial.println("FMTuner stop ...");
    _radio->powerDown();
}

void FMTuner4735::DisplayInfo()
{
    Serial.println("Radio info:");
    _radio->getStatus();
    _radio->getCurrentReceivedSignalQuality();
    Serial.print("You are tuned on ");
    if (_radio->isCurrentTuneFM())
    {
        Serial.print(String(currentFrequency / 100.0, 2));
        Serial.print("MHz ");
        Serial.print((_radio->getCurrentPilot()) ? "STEREO" : "MONO");
    }
    else
    {
        Serial.print(currentFrequency);
        Serial.print("kHz");
    }
    Serial.print(" [SNR:");
    Serial.print(_radio->getCurrentSNR());
    Serial.print("dB");

    Serial.print(" Signal:");
    Serial.print(_radio->getCurrentRSSI());
    Serial.println("dBuV]");

    Serial.println("Volume: " + String(_volume));
    
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
    _current_station_preset = num;
    currentFrequency = _station_presets[num];
    _radio->setFrequency(currentFrequency);
    DisplayInfo();
}

void FMTuner4735::SaveCurrentChannel(int preset)
{
    Serial.println("Saving frequency " + String(currentFrequency)+ " as preset " + preset);
    _station_presets[preset] = currentFrequency;

    SavePresets();
}

void FMTuner4735::SavePresets()
{
    Preferences _prefs;
    _prefs.begin("esp32radio_fmtuner", false); 
    _prefs.putUShort("FMPRESET_0", _station_presets[0]);
    _prefs.putUShort("FMPRESET_1", _station_presets[1]);
    _prefs.putUShort("FMPRESET_2", _station_presets[2]);
    _prefs.putUShort("FMPRESET_3", _station_presets[3]);
    _prefs.putUShort("FMPRESET_4", _station_presets[4]);
    _prefs.putUShort("FMPRESET_5", _station_presets[5]);
    _prefs.putUShort("FMPRESET_6", _station_presets[6]);
    _prefs.putUShort("FMPRESET_7", _station_presets[7]);
    _prefs.putUShort("LASTPRESET", _current_station_preset);
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
            _radio->frequencyUp();
            channelchanged = true;
            break;
        case 'i':
            _radio->frequencyDown();
            channelchanged = true;
            break;
        case 'O':
            _radio->setFrequency(currentFrequency + 25);          
            channelchanged = true;
            break;
        case 'I':
            _radio->setFrequency(currentFrequency - 25);
            channelchanged = true;
            break;
        case 'u':
            _seekmode = ch;
            _radio->seekStation(1, 1);
            _seektimer = millis();
            channelchanged = true;
            break;
        case 'z':
            _seekmode = ch;
            _radio->seekStation(0, 1);
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
            _radio->volumeUp();
            _volume = _radio->getVolume();
            DisplayInfo();
            break;
        case '-':
            _radio->volumeDown();
            _volume = _radio->getVolume();
            DisplayInfo();
            break;
    }

    if (ch == 'r')
    {

            char *block0A = _radio->getRdsText0A();
            char *block2A = _radio->getRdsText2A();
            char *block2B = _radio->getRdsText2B();
            char *time = _radio->getRdsTime();
            char *text = _radio->getRdsText();

            if(_radio->getRdsSync())
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
        currentFrequency = _radio->getFrequency();
        _radio->setVolume(_volume);

        if(_savemode)
            SaveCurrentChannel(_current_station_preset);

        DisplayInfo();
    }
}