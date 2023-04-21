#include "fmtuner4735.hpp"

FMTuner4735::FMTuner4735(DACIndicator *freq, DACIndicator *signal)
{
    Serial.println("Start Si4735 ...");
    
    _pwmindicator_freq = freq;
    _pwmindicator_signal = signal;
    _pwmindicator_signal->SetRange(0, 127);

    digitalWrite(SI7435_RESET_PIN, HIGH);
    Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);
   
    _radio = new SI4735();
    _radio->getDeviceI2CAddress(SI7435_RESET_PIN);

    //_radio->setMaxDelaySetFrequency(50);
    _radio->setTuneFrequencyAntennaCapacitor(0);

    _radio->setup(SI7435_RESET_PIN,0,FM_FUNCTION, SI473X_ANALOG_AUDIO, XOSCEN_CRYSTAL, 0);

    _radio->setFmSoftMuteMaxAttenuation(0);
    _radio->setAmSoftMuteMaxAttenuation(0);

    _radio->setFmStereoOn();
    _radio->setFMDeEmphasis(2);
    _radio->RdsInit();
    delay(100);
    _lastRDSUpdate = _lastUpdate = millis();
}

void FMTuner4735::Start(uint8_t band)
{
    Serial.println("FMTuner start ...");

    Band b = _bands[band]; 

    Serial.println("Band " + String(b.bandName));

    // Initialize the Radio
    //_radio->radioPowerUp();
    //digitalWrite(SI7435_RESET_PIN, HIGH);
    //Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);
    //_radio->setup(SI7435_RESET_PIN,0, b.bandType, SI473X_ANALOG_AUDIO, 0, 0);

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
    Serial.println("Switch to band " + bandIdx);
    if(bandIdx == _currentBand)
    {
        Serial.println("Band already active!");
        return;
    }

    Band b = _bands[bandIdx];

    if(_currentBand != -1)
    {
        SavePresets();
    }

    if (b.bandType == FM_BAND_TYPE)
    {
        Serial.println("... FM Mode band " + String(b.bandName));
        currentMode = FM;
        
        _radio->setFM(b.minimumFreq, b.maximumFreq, b.currentFreq, tabFmStep[b.currentStepIdx]);
        
        _radio->setRdsConfig(1, 0,0,0,0);
        _radio->setFifoCount(1);
        
        
        /*char str[200];
        sprintf(str,"Pos %2.2d | Min %2.2d | Max %2.2d | Step %2.2d | Bw %2.2d |disableAgc %2.2d  | agcIdx %2.2d | agcNdx %2.2d | avcIdx %2.2d", bandIdx, b.minimumFreq, b.maximumFreq, tabFmStep[b.currentStepIdx], bandwidthFM[b.bandwidthIdx].idx, b.disableAgc, b.agcIdx, b.agcNdx, b.avcIdx );
        Serial.println(str);*/

        _smallstep = tabFmStep[0];
        _step = tabFmStep[1];
    }
    else
    {
        Serial.println("... AM Mode band " + String(b.bandName));
        b.disableAgc = b.disableAgc;

        // char str[100];
        // sprintf(str,"Pos %2.2d | disableAgc %2.2d  | agcIdx %2.2d | agcNdx %2.2d | avcIdx %2.2d", bandIdx, disableAgc, agcIdx, agcNdx, avcIdx );
        // Serial.println(str);

        
        // set the tuning capacitor for SW or MW/LW
        _radio->setTuneFrequencyAntennaCapacitor((b.bandType == MW_BAND_TYPE || b.bandType == LW_BAND_TYPE) ? 0 : 1);

        currentMode = AM;
        _radio->setAM(b.minimumFreq, b.maximumFreq, b.currentFreq, tabAmStep[b.currentStepIdx]);

        _radio->setBandwidth(bandwidthAM[b.bandwidthIdx].idx, 1);
        _radio->setAmSoftMuteMaxAttenuation(b.softMuteMaxAttenuation); // Soft Mute for AM or SSB
        _radio->setAutomaticGainControl(b.disableAgc, b.agcNdx);

        
        _radio->setSeekAmLimits(b.minimumFreq, b.maximumFreq); // Consider the range all defined current band
        _radio->setSeekAmSpacing(5); // Max 10kHz for spacing
        _radio->setAvcAmMaxGain(b.avcIdx);

        _smallstep = tabAmStep[0];
        _step = tabAmStep[1];
    }
    delay(100);
    currentFrequency = b.currentFreq;
    currentStepIdx = b.currentStepIdx;
    _currentBand = bandIdx;

    _pwmindicator_freq->SetRange(b.minimumFreq, b.maximumFreq);

    LoadPresets();
}

uint16_t FMTuner4735::GetBandMin()
{
    return _bands[_currentBand].minimumFreq;
}

uint16_t FMTuner4735::GetBandMax()
{
    return _bands[_currentBand].maximumFreq;
}

void FMTuner4735::setFrequency(u_int16_t freq)
{
    Band b = _bands[_currentBand];
    if(freq < b.minimumFreq)
        freq = b.minimumFreq;

    if(freq > b.maximumFreq)
        freq = b.maximumFreq;

    _radio->setFrequency(freq);
    _pwmindicator_freq->SetValue(currentFrequency);
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

    uint8_t offset = _radio->getCurrentSignedFrequencyOffset();
    Serial.println(" SignedFrequencyOffset: " + String(offset) + " kHz");

    Serial.println("Volume: " + String(_volume));
    
    if(_savemode)
        Serial.println("Preset save mode is: on");
    else
        Serial.println("Preset save mode is: off");

    
}

String FMTuner4735::GetFreqDisplayText()
{
    if (_radio->isCurrentTuneFM())
        return String((double)currentFrequency / 100.0, 2);


    return String(currentFrequency);
}

String FMTuner4735::GetClockDisplayText()
{
    if(clockdisplaypage == 1 && RDSMessage.length() > 0)
    {
        return RDSMessage;
    }


    uint8_t rssi = _radio->getCurrentRSSI();
    return GetFreqDisplayText() + "  RSSI: " + String(rssi) + " ";
}

void FMTuner4735::SetSaveMode(bool onoff)
{
    _savemode = onoff;
}

void FMTuner4735::SwitchPreset(uint8_t num)
{
    Serial.println("FMTuner::SwitchPreset to " + String(num));
    _current_station_preset = num;
    currentFrequency = _station_presets[num];
    setFrequency(currentFrequency);
    DisplayInfo();
}

void FMTuner4735::SaveCurrentChannel(uint8_t preset)
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
        Serial.println("Could not open preset file " + prefname);
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

String UpdateRDSMessage(String oldmsg, char * newmsg)
{
    String newstr = String(newmsg);
    newstr.trim();

    if(oldmsg.length() == 0)
        return newstr;

    if(newstr.length() > oldmsg.length())
        return newstr;

    if(oldmsg.endsWith(newstr) || oldmsg.startsWith(newstr))
        return oldmsg;

    return newstr;
}

void FMTuner4735::checkRDS()
{
  _radio->getRdsStatus();
  if (_radio->getRdsReceived())
  {
    if (_radio->getRdsSync() && _radio->getRdsSyncFound())
    {
      rdsMsg = _radio->getRdsText2A();
      stationName = _radio->getRdsText0A();
      rdsTime = _radio->getRdsTime();

      String msg = UpdateRDSMessage(RDSMessage, rdsMsg);
      if(msg != RDSMessage && msg != "")
      {
        RDSMessage = msg;
        Serial.println("RDS Message: '" + RDSMessage + "'");
      }

      String stationname = UpdateRDSMessage(RDSStationName, stationName);
      if(stationname != RDSStationName && stationname != "")
      {
        RDSStationName = stationname;
        Serial.println("RDS Stationname: '" + RDSStationName+ "'");
      }

      String time = UpdateRDSMessage(RDSTime, rdsTime);
      if(time != RDSTime && time != "")
      {
        RDSTime = time;
        Serial.println("RDS Time: '" + RDSTime+ "'");
      }
    }
  }
}

void FMTuner4735::Loop(char ch)
{
    bool channelchanged = false;
    long now = millis();

    if((now - clockdisplaypagetimer) > 15000)
    {
        clockdisplaypage++;
        if(clockdisplaypage == 2)
            clockdisplaypage = 0;
        
        clockdisplaypagetimer = now;
    }

    if(now - _lastUpdate > 100)
    {
        _radio->getStatus();
        _radio->getCurrentReceivedSignalQuality();
        _pwmindicator_signal->SetValue(_radio->getCurrentRSSI());
        _lastUpdate = now;
    }

    if(_seekmode != '0' && ch != 't')
    {
        delay(100);
        _radio->getCurrentReceivedSignalQuality();
        uint8_t rssi = _radio->getCurrentRSSI();
        uint8_t snr = _radio->getCurrentSNR();
        if(rssi < 24 || (now - _seektimer) > 4000)
        {
            ch = _seekmode;
            char str[100];
            sprintf(str,"Seeking .... Freq: %2.2d | RSSI: %2.2d | SNR: %2.2d", currentFrequency, rssi, snr);
            Serial.println(str);
        }

    }

    uint16_t newfreq = 0;

    switch(ch)
    {
        case 'o':
            newfreq = currentFrequency + _smallstep;
            setFrequency(newfreq);  
            _seekmode = '0';
            channelchanged = true;
            Serial.println("Frequency up " + String(_smallstep));
            break;
        case 'i':
            newfreq = currentFrequency - _smallstep;
            setFrequency(newfreq);  
            _seekmode = '0';
            channelchanged = true;
            Serial.println("Frequency down " + String(_smallstep));
            break;
        case 'I':
            newfreq = currentFrequency + _step;
            setFrequency(newfreq);          
            _seekmode = '0';
            channelchanged = true;
            Serial.println("Frequency up " + String(_step));
            break;
        case 'O':
            newfreq = currentFrequency - _step;
            setFrequency(newfreq);
            _seekmode = '0';
            channelchanged = true;
            Serial.println("Frequency down " + String(_step));
            break;
        case 'u':
            _seekmode = ch;
            _radio->frequencyUp();
            _seektimer = millis();
            channelchanged = true;
            Serial.println("Start seek up");
            break;
        case 'z':
            _seekmode = ch;
            _radio->frequencyDown();
            Serial.println("Start seek down");
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
        if(rdsMsg != NULL && rdsMsg[0] != '\0')
            Serial.println("Msg: " + String(rdsMsg));

        if(stationName != NULL && stationName[0] != '\0')    
            Serial.println("Station: " + String(stationName));

        if(rdsTime != NULL && rdsTime[0] != '\0')
            Serial.println("Time" + String(rdsTime));

    }

    currentFrequency = _radio->getCurrentFrequency();
    if(previousFrequency != currentFrequency) 
    {
        Serial.println("Frequency changed!");
        previousFrequency = currentFrequency;
        
        if(_savemode)
            SaveCurrentChannel(_current_station_preset);

        DisplayInfo();
        RDSMessage.clear();
        RDSStationName.clear();
        RDSTime.clear();
        
    }

    if(_bands[_currentBand].bandType == FM_BAND_TYPE)
    {
        if(now - _lastRDSUpdate > 10)
        {
            checkRDS();
            _lastRDSUpdate = now;
        }
    }
}