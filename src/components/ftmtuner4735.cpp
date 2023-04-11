#include "fmtuner4735.hpp"

SI4735 *rx = NULL;

FMTuner4735::FMTuner4735()
{
    Serial.println("Start Si4735 ...");
    
+
    Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);
   
    _radio = new SI4735();
    rx = _radio;
    _radio->getDeviceI2CAddress(SI7435_RESET_PIN);
    _radio->setMaxDelayPowerUp(500);
   
    _radio->setup(SI7435_RESET_PIN,0, FM_FUNCTION, SI473X_ANALOG_AUDIO, 0, 0);

    delay(100);
}

void FMTuner4735::Start(uint8_t band)
{
    Serial.println("FMTuner start ...");

    Band b = _bands[band]; 

    // Initialize the Radio
    //_radio->radioPowerUp();
    digitalWrite(SI7435_RESET_PIN, HIGH);
    Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);
    _radio->setMaxDelaySetFrequency(150);
    _radio->setMaxDelayPowerUp(500);
    _radio->setup(SI7435_RESET_PIN,0, b.bandType, SI473X_ANALOG_AUDIO, 1, 0);

    _radio->setSeekAmRssiThreshold(50);
    _radio->setSeekAmSrnThreshold(20);
    _radio->setSeekFmRssiThreshold(5);
    _radio->setSeekFmSrnThreshold(5);
    _radio->setFmSoftMuteMaxAttenuation(0);
    _radio->setAmSoftMuteMaxAttenuation(0);
    _radio->setTuneFrequencyAntennaCapacitor(0);
    
    _radio->setFMDeEmphasis(2);
    _radio->RdsInit();
    
    SwitchBand(band);

    _radio->setVolume(_volume);
    
    DisplayInfo();
}

void FMTuner4735::Stop()
{
    Serial.println("FMTuner stop ...");
    //_radio->powerDown();
}

void FMTuner4735::SwitchBand(uint8_t bandIdx)
{
    Band b = _bands[bandIdx];

    Serial.println("Switch to band " + String(b.bandName));

    if(bandIdx == _currentBand)
    {
        Serial.println("Band already active!");
        return;
    }

    if(_currentBand != -1)
    {
        SavePresets();
    }

    if (b.bandType == FM_BAND_TYPE)
    {
        Serial.println("... FM Mode band " + String(b.bandName));
        currentMode = FM;
        _radio->setTuneFrequencyAntennaCapacitor(0);
        _radio->setFM(b.minimumFreq, b.maximumFreq, b.currentFreq, tabFmStep[b.currentStepIdx]);
        _radio->setSeekFmLimits(b.minimumFreq, b.maximumFreq);
        
        _radio->setRdsConfig(1, 2, 2, 2, 2);
        _radio->setFifoCount(1);
        
        bwIdxFM = b.bandwidthIdx;
        _radio->setFmBandwidth(bandwidthFM[bwIdxFM].idx);    
        _radio->setAutomaticGainControl(disableAgc, agcNdx);
        _radio->setFmSoftMuteMaxAttenuation(softMuteMaxAttIdx);
    }
    else
    {
        Serial.println("... AM Mode band " + String(b.bandName));
        disableAgc = b.disableAgc;
        agcIdx = b.agcIdx;
        agcNdx = b.agcNdx;
        avcIdx = b.avcIdx;

        // char str[100];
        // sprintf(str,"Pos %2.2d | disableAgc %2.2d  | agcIdx %2.2d | agcNdx %2.2d | avcIdx %2.2d", bandIdx, disableAgc, agcIdx, agcNdx, avcIdx );
        // Serial.println(str);

        
        // set the tuning capacitor for SW or MW/LW
        _radio->setTuneFrequencyAntennaCapacitor((b.bandType == MW_BAND_TYPE || b.bandType == LW_BAND_TYPE) ? 0 : 1);

        currentMode = AM;
        _radio->setAM(b.minimumFreq, b.maximumFreq, b.currentFreq, tabAmStep[b.currentStepIdx]);

        bwIdxAM = b.bandwidthIdx;
        _radio->setBandwidth(bandwidthAM[bwIdxAM].idx, 1);
        _radio->setAmSoftMuteMaxAttenuation(softMuteMaxAttIdx); // Soft Mute for AM or SSB
        _radio->setAutomaticGainControl(disableAgc, agcNdx);

        
        _radio->setSeekAmLimits(b.minimumFreq, b.maximumFreq); // Consider the range all defined current band
        _radio->setSeekAmSpacing(5); // Max 10kHz for spacing
        _radio->setAvcAmMaxGain(avcIdx);
    }
    delay(100);
    currentFrequency = b.currentFreq;
    currentStepIdx = b.currentStepIdx;
    _currentBand = bandIdx;
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

String FMTuner4735::GetFreqDisplayText()
{
    if (_radio->isCurrentTuneFM())
    {
        return String(currentFrequency / 100.0, 2) + "MHz";
    }
    
    return String(currentFrequency) + "kHz";
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

void FMTuner4735::LoadPresets()
{
    Serial.println("Load presets");
    if(_currentBand < 0 || _currentBand > 7)
    {
        Serial.println(" ... invalid band");
        return; 
    }    

    
    Band b = _bands[_currentBand];
    Preferences _prefs;
    String prefname = "radio_tuner_" + String(b.bandName);
    if(!_prefs.begin(prefname.c_str(), false)) 
    {
        Serial.println("unable to open preferences");
    }
    else
    {
        Serial.println("Loading presets from SPIIFFS");
        _station_presets[0] = _prefs.getUShort("PRESET_0", b.currentFreq);
        _station_presets[1] = _prefs.getUShort("PRESET_1", 0);
        _station_presets[2] = _prefs.getUShort("PRESET_2", 0);
        _station_presets[3] = _prefs.getUShort("PRESET_3", 0);
        _station_presets[4] = _prefs.getUShort("PRESET_4", 0);
        _station_presets[5] = _prefs.getUShort("PRESET_5", 0);
        _station_presets[6] = _prefs.getUShort("PRESET_6", 0);
        _station_presets[7] = _prefs.getUShort("PRESET_7", 0);
        _current_station_preset = _prefs.getUShort("LASTPRESET", 0);
        _prefs.end();

        currentFrequency = _station_presets[_current_station_preset];
    }
}

void FMTuner4735::SavePresets()
{
    Serial.println("Save presets band " + String(_currentBand));
    if(_currentBand < 0 || _currentBand > 7)
    {
        Serial.println(" ... invalid band");
        return; 
    } 

    Band b = _bands[_currentBand];

    Preferences _prefs;
    String prefname = "radio_tuner_" + String(b.bandName);
    Serial.println(prefname);
    if(!_prefs.begin(prefname.c_str(), false))
    {
        Serial.println("Could not open reset file " + prefname);
        return;
    }
    _prefs.putUShort("PRESET_0", _station_presets[0]);
    _prefs.putUShort("PRESET_1", _station_presets[1]);
    _prefs.putUShort("PRESET_2", _station_presets[2]);
    _prefs.putUShort("PRESET_3", _station_presets[3]);
    _prefs.putUShort("PRESET_4", _station_presets[4]);
    _prefs.putUShort("PRESET_5", _station_presets[5]);
    _prefs.putUShort("PRESET_6", _station_presets[6]);
    _prefs.putUShort("PRESET_7", _station_presets[7]);
    _prefs.putUShort("LASTPRESET", _current_station_preset);
    _prefs.end();
}



void showFrequency(uint16_t freq ) {

    if(rx == NULL)
        return;

    if (rx->isCurrentTuneFM())
    {
        Serial.print(String(freq / 100.0, 2));
        Serial.println("MHz ");
    }
    else
    {
        Serial.print(freq);
        Serial.println("kHz");
    }
  
}

void FMTuner4735::Loop(char ch)
{
    bool channelchanged = false;
    long now = millis();

    if(_seekmode != '0' && (now - _seektimer) > 2000)
    {
        Serial.println("Seek still going on ....");
        //ch = _seekmode;
    }

    uint16_t newfreq = 0;

    switch(ch)
    {
        case 'o':
            _radio->frequencyUp();
            channelchanged = true;
            Serial.println("Frequency up");
            break;
        case 'i':
            _radio->frequencyDown();
            channelchanged = true;
            Serial.println("Frequency down");
            break;
        case 'O':
            newfreq = currentFrequency + 25;
            _radio->setFrequency(newfreq);          
            channelchanged = true;
            break;
        case 'I':
            newfreq = currentFrequency - 25;
            _radio->setFrequency(newfreq);
            channelchanged = true;
            break;
        case 'u':
            //_seekmode = ch;
            //_radio->seekStation(1, 1);
            _radio->seekStationProgress(showFrequency,1);
            _seektimer = millis();
            channelchanged = true;
            Serial.println("Start seek up");
            break;
        case 'z':
            //_seekmode = ch;
            Serial.println("Start seek down");
            _radio->seekStationProgress(showFrequency,0);
            //_radio->seekStation(0, 1);
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

    currentFrequency = _radio->getCurrentFrequency();
    if(previousFrequency != currentFrequency) 
    {
        delay(100);
        previousFrequency = currentFrequency;
        
        _radio->setVolume(0);
        _radio->setVolume(30);
        _radio->setVolume(_volume);

        if(_savemode)
            SaveCurrentChannel(_current_station_preset);

        DisplayInfo();
    }
}