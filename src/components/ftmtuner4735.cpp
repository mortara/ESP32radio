#include "fmtuner4735.hpp"
#include "MQTT/mqtt.hpp"
#include "rotary_encoder.hpp"
#include "tunerbuttons.hpp"

FMTuner4735::FMTuner4735()
{
    pmLogging.LogLn("Start Si4735 ...");
    
    pinMode(SI7435_RESET_PIN, OUTPUT);
    digitalWrite(SI7435_RESET_PIN, HIGH);
    //Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL, 10000);

    
    _radio = new SI4735();
    uint8_t adr = _radio->getDeviceI2CAddress(SI7435_RESET_PIN);
    if(adr == 0)
    {
        pmLogging.LogLn("Device address: " + String(adr));
        pmLogging.LogLn("Si4735 not found!" );
        return;
    }
    
    //_radio->setMaxDelaySetFrequency(50);
    _radio->setTuneFrequencyAntennaCapacitor(0);
    _radio->setup(SI7435_RESET_PIN,0,FM_FUNCTION, SI473X_ANALOG_AUDIO, XOSCEN_CRYSTAL, 0);

    _radio->setFmSoftMuteMaxAttenuation(0);
    _radio->setAmSoftMuteMaxAttenuation(0);

    _radio->setFmStereoOn();
    _radio->setFMDeEmphasis(2);
    _radio->RdsInit();

    _lastRotaryCount = RotaryEncoder.GetCounter();
    _active = true;
    _lastRotaryRead = frequencychangetimeout = _lastRDSUpdate = _lastSignalUpdate = millis();
}

void FMTuner4735::setupMQTT()
{
    if(mqttsetup)
        return;

    pmLogging.LogLn("Setting up si4735 MQTT client");

    if(!pmCommonLib.MQTTConnector.SetupSensor("Frequency", "SI4735", "frequency", "Hz", "mdi:sine-wave"))
    {
        pmLogging.LogLn("Unable to setup si4735 MQTT client");            
        return;
    }
    
    pmCommonLib.MQTTConnector.SetupSensor("Band", "SI4735", "", "", "mdi:radio");
    pmCommonLib.MQTTConnector.SetupSensor("BandLowerLimit", "SI4735", "frequency", "Hz", "mdi:sine-wave");
    pmCommonLib.MQTTConnector.SetupSensor("BandHighLimit", "SI4735", "frequency", "Hz", "mdi:sine-wave");
    pmCommonLib.MQTTConnector.SetupSensor("RSSI", "SI4735", "signal_strength", "dB", "mdi:radio-tower");
    pmCommonLib.MQTTConnector.SetupSensor("SNR", "SI4735", "signal_strength", "dB", "mdi:radio-tower");
    pmCommonLib.MQTTConnector.SetupSensor("RDSMSG", "SI4735", "", "", "mdi:message-processing");
    pmCommonLib.MQTTConnector.SetupSensor("BandData", "SI4735", "", "", "mdi:radio-tower");

    pmCommonLib.MQTTConnector.SetupButton("StartSeek", "SI4735", "", "mdi:radio-tower");
    pmCommonLib.MQTTConnector.SetupButton("StopSeek", "SI4735", "", "mdi:radio-tower");

    pmLogging.LogLn("si4735 mqtt setup done!");

    mqttsetup = true;
}

void FMTuner4735::sendMQTTState()
{
    Band b = _bands[_currentBand];

    JsonDocument payload;
    payload["Frequency"] = _radio->getCurrentFrequency();
    payload["RSSI"] = _radio->getCurrentRSSI();
    payload["SNR"] = _radio->getCurrentSNR();
    payload["Band"] = b.bandName;
    payload["BandLowerLimit"] = b.minimumFreq;
    payload["BandHighLimit"] = b.maximumFreq;
    payload["Preset"] = _current_station_preset;
    payload["RDSMSG"] = RDSMessage;

    char str[200];
    if (b.bandType == FM_BAND_TYPE)
        sprintf(str,"Smallstep %2.2d | Step %2.2d | Bw %2.2d | disableAgc %2.2d  | agcIdx %2.2d | agcNdx %2.2d | avcIdx %2.2d", tabFmStep[0], tabFmStep[b.currentStepIdx], bandwidthFM[b.bandwidthIdx].idx, b.disableAgc, b.agcIdx, b.agcNdx, b.avcIdx );
    else
        sprintf(str,"Smallstep %2.2d | Step %2.2d | Bw %2.2d | disableAgc %2.2d  | agcIdx %2.2d | agcNdx %2.2d | avcIdx %2.2d", tabAmStep[0], tabAmStep[b.currentStepIdx], bandwidthAM[b.bandwidthIdx].idx, b.disableAgc, b.agcIdx, b.agcNdx, b.avcIdx );

    payload["BandData"] = String(str);

    pmCommonLib.MQTTConnector.PublishMessage(payload, "SI4735");

    _lastMQTTUpdate = millis();
}

uint8_t FMTuner4735::Start(uint8_t band)
{
    if(!_active || _radio == NULL)
        return 0;

    pmLogging.LogLn("FMTuner start ...");

    _radio->radioPowerUp();
    SignalIndicator.SetRange(0, 127);
    
    SwitchBand(band);
    _radio->setVolume(_volume);
    
    SwitchPreset(_current_station_preset);

    DisplayInfo();

    return _current_station_preset;
}

void FMTuner4735::Stop()
{
    pmLogging.LogLn("FMTuner stop ...");
    _radio->powerDown();
}

void FMTuner4735::SwitchBand(uint8_t bandIdx)
{
    if(!_active)
        return;
        
    _seekmode = '0';
    pmLogging.LogLn("Switch to band " + String(bandIdx));
    if(bandIdx == _currentBand)
    {
        pmLogging.LogLn("Band already active!");
        return;
    }

    Band b = _bands[bandIdx];

    if(_currentBand != -1)
    {
        SavePresets();
    }

    if (b.bandType == FM_BAND_TYPE)
    {
        pmLogging.LogLn("... FM Mode band " + String(b.bandName));
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
        pmLogging.LogLn("... AM Mode band " + String(b.bandName));

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
    delay(50);

    currentStepIdx = b.currentStepIdx;
    _currentBand = bandIdx;

    FrequencyIndicator.SetRange(b.minimumFreq, b.maximumFreq);

    LoadPresets();

    SwitchPreset(_current_station_preset);

    sendMQTTState();
}

uint16_t FMTuner4735::GetBandMin()
{
    return _bands[_currentBand].minimumFreq;
}

uint16_t FMTuner4735::GetBandMax()
{
    return _bands[_currentBand].maximumFreq;
}

uint16_t FMTuner4735::setFrequency(u_int16_t freq)
{
    if(!_active)
        return currentFrequency;

    pmLogging.Log("Set frequency to " + String(freq) + " ... ");    

    Band b = _bands[_currentBand];
    uint16_t fwidth = b.maximumFreq - b.minimumFreq;

    while(freq < b.minimumFreq)
        freq += fwidth;

    while(freq > b.maximumFreq)
        freq -= fwidth;

    _radio->setFrequency(freq);
    currentFrequency = _radio->getCurrentFrequency();
    FrequencyIndicator.SetValue(currentFrequency);

    return currentFrequency;
}

void FMTuner4735::DisplayInfo()
{
    pmLogging.LogLn("Radio info:");

    if(!_active)
    {
        pmLogging.LogLn(" -- inactive --");
        return;
    }
    
    pmLogging.Log("You are tuned on ");
    if (_radio->isCurrentTuneFM())
    {
        pmLogging.Log(String(currentFrequency / 100.0, 2));
        pmLogging.Log("MHz ");
        pmLogging.Log((_radio->getCurrentPilot()) ? "STEREO" : "MONO");
    }
    else
    {
        pmLogging.Log(String(currentFrequency));
        pmLogging.Log("kHz");
    }
    pmLogging.Log(" [SNR:");
    pmLogging.Log(String(_radio->getCurrentSNR()));
    pmLogging.Log("dB");

    pmLogging.Log(" Signal:");
    pmLogging.Log(String(_radio->getCurrentRSSI()));
    pmLogging.LogLn("dBuV]");

    uint8_t offset = _radio->getCurrentSignedFrequencyOffset();
    pmLogging.LogLn(" SignedFrequencyOffset: " + String(offset) + " kHz");

    pmLogging.LogLn("Volume: " + String(_volume));
    
    if(TunerButtons.SavePresetButtonPressed)
        pmLogging.LogLn("Preset save mode is: on");
    else
        pmLogging.LogLn("Preset save mode is: off");

    if(rdsMsg != NULL && rdsMsg[0] != '\0')
        pmLogging.LogLn("RDS Msg: " + String(rdsMsg));

    if(stationName != NULL && stationName[0] != '\0')    
        pmLogging.LogLn("RDS Station: " + String(stationName));

    if(rdsTime != NULL && rdsTime[0] != '\0')
        pmLogging.LogLn("RDS Time" + String(rdsTime));

    pmLogging.LogLn("Presets for band " + String(_currentBand));
    
    Band b = _bands[_currentBand];
    Preferences _prefs;
    String prefname = "radio_tuner_" + String(b.bandName);
    if(!_prefs.begin(prefname.c_str(), false)) 
    {
        pmLogging.LogLn("unable to open preferences");
    }
    else
    {
        pmLogging.LogLn("Preset 0: " + String(_prefs.getUShort("PRESET_0")));
        pmLogging.LogLn("Preset 1: " + String(_prefs.getUShort("PRESET_1")));
        pmLogging.LogLn("Preset 2: " + String(_prefs.getUShort("PRESET_2")));
        pmLogging.LogLn("Preset 3: " + String(_prefs.getUShort("PRESET_3")));
        pmLogging.LogLn("Preset 4: " + String(_prefs.getUShort("PRESET_4")));
        pmLogging.LogLn("Preset 5: " + String(_prefs.getUShort("PRESET_5")));
        pmLogging.LogLn("Preset 6: " + String(_prefs.getUShort("PRESET_6")));
        pmLogging.LogLn("Preset 7: " + String(_prefs.getUShort("PRESET_7")));
    }

    _prefs.end();
}

String FMTuner4735::GetFreqDisplayText()
{
    if(!_active)
        return "No tuner!";

    if (_radio->isCurrentTuneFM())
        return String((double)currentFrequency / 100.0, 2);


    return String(currentFrequency);
}

String FMTuner4735::GetClockDisplayText()
{
    if(!_active || _radio == NULL)
    {
        return "Tuner not found!";
    }

    if(clockdisplaypage == 1 || _seekmode != '0')
    {
        return RDSMessage;
    }

    if(clockdisplaypage > 1)
    {
        return Menu.GetLine(-(clockdisplaypage-1), 0);
    }


    uint8_t rssi = _radio->getCurrentRSSI();
    String t = GetFreqDisplayText() + "  RSSI: " + String(rssi) + " ";
    if(t.length() > 16)
        t = t.substring(0,16);

    return t;
}

void FMTuner4735::SwitchPreset(uint8_t num)
{
    pmLogging.LogLn("FMTuner::SwitchPreset to " + String(num));
    _current_station_preset = num;
    currentFrequency = setFrequency(_station_presets[num]);
    _seekmode = '0';
}

void FMTuner4735::SaveCurrentChannel(uint8_t preset)
{
    pmLogging.LogLn("Saving frequency " + String(currentFrequency)+ " as preset " + preset);
    _station_presets[preset] = currentFrequency;

    SavePresets();
}

void FMTuner4735::LoadPresets()
{
    pmLogging.LogLn("Load presets for band " + String(_currentBand));
    if(_currentBand < 0 || _currentBand > 6)
    {
        pmLogging.LogLn(" ... invalid band");
        return; 
    }    

    Band b = _bands[_currentBand];
    Preferences _prefs;
    String prefname = "radio_tuner_" + String(b.bandName);
    if(!_prefs.begin(prefname.c_str(), false)) 
    {
        pmLogging.LogLn("unable to open preferences");
    }
    else
    {
        pmLogging.LogLn("Loading presets from SPIFFS");
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
    pmLogging.LogLn("Save presets band " + String(_currentBand));
    if(_currentBand < 0 || _currentBand > 7)
    {
        pmLogging.LogLn(" ... invalid band");
        return; 
    } 

    Band b = _bands[_currentBand];

    Preferences _prefs;
    String prefname = "radio_tuner_" + String(b.bandName);
    pmLogging.LogLn(prefname);
    if(!_prefs.begin(prefname.c_str(), false))
    {
        pmLogging.LogLn("Could not open preset file " + prefname);
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
    if(!_active)
        return;

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
                //pmLogging.LogLn("RDS Message: '" + RDSMessage + "'");
            }

            String stationname = UpdateRDSMessage(RDSStationName, stationName);
            if(stationname != RDSStationName && stationname != "")
            {
                RDSStationName = stationname;
                //pmLogging.LogLn("RDS Stationname: '" + RDSStationName+ "'");
            }

            String time = UpdateRDSMessage(RDSTime, rdsTime);
            if(time != RDSTime && time != "")
            {
                RDSTime = time;
                //pmLogging.LogLn("RDS Time: '" + RDSTime+ "'");
            }
        }
    }
}

void FMTuner4735::Loop(char ch)
{
    if(!_active)
        return;
    unsigned long now = millis();

    if((now - clockdisplaypagetimer) > 15000UL)
    {
        clockdisplaypage++;
        if(clockdisplaypage == 2 + Menu.InfoPages)
            clockdisplaypage = 0;
        
        clockdisplaypagetimer = now;  
    }

    if(now - _lastMQTTUpdate > 5000UL)
    {
        if(mqttsetup)
        {   
            sendMQTTState();
        } else if(pmCommonLib.MQTTConnector.isActive())
            setupMQTT();
    }

    if(now - _lastSignalUpdate > 1000UL)
    {
        _radio->getCurrentReceivedSignalQuality();
        SignalIndicator.SetValue(_radio->getCurrentRSSI());
        _lastSignalUpdate = now;
    }

    if(_seekmode != '0' && ch != 't')
    {
        delay(100);
        _radio->getCurrentReceivedSignalQuality();
        uint8_t rssi = _radio->getCurrentRSSI();
        uint8_t snr = _radio->getCurrentSNR();
        if(rssi < 23 || (now - _seektimer) > 6000UL)
        {
            ch = _seekmode;
            char str[100];
            sprintf(str,"Seeking .... Freq: %2.2d | RSSI: %2.2d | SNR: %2.2d", currentFrequency, rssi, snr);
            pmLogging.LogLn(str);
        }

    }

    if(now - _lastRotaryRead > 100UL)
    {
        _lastRotaryRead = now;
        int64_t rotary = RotaryEncoder.GetCounter();
        if(rotary != _lastRotaryCount)
        {
            float diff = (float)(rotary - _lastRotaryCount) / 5.0;
            if(abs(diff) < 1.0)
                if(diff > 0)
                    diff = 1;
                else if(diff < 0)
                    diff = -1;
                else
                    diff = 0;

            
            _lastRotaryCount = rotary;
            _seekmode = '0';
            setFrequency((int)currentFrequency + (int)((int)_smallstep * (int)diff));
            frequencychangetimeout = now + 500;
            return;
        }
        _lastRotaryCount = rotary;
    }
    
    bool channelchanged = false;
    switch(ch)
    {
        case 'o':
            setFrequency(currentFrequency + _smallstep); 
            _seekmode = '0';
            channelchanged = true;
            pmLogging.LogLn("Frequency up " + String(_smallstep));
            break;
        case 'i':
            setFrequency(currentFrequency - _smallstep);
            _seekmode = '0';
            channelchanged = true;
            pmLogging.LogLn("Frequency down " + String(_smallstep));
            break;
        case 'I':
            setFrequency(currentFrequency + _step);     
            _seekmode = '0';
            channelchanged = true;
            pmLogging.LogLn("Frequency up " + String(_step));
            break;
        case 'O':
            setFrequency(currentFrequency - _step);
            _seekmode = '0';
            channelchanged = true;
            pmLogging.LogLn("Frequency down " + String(_step));
            break;
        case 'u':
            _seekmode = ch;
            setFrequency(currentFrequency + _smallstep);
            _seektimer = millis();
            channelchanged = true;
            pmLogging.LogLn("Start seek up");
            break;
        case 'z':
            _seekmode = ch;
            setFrequency(currentFrequency - _smallstep);
            pmLogging.LogLn("Start seek down");
            _seektimer = millis();
            channelchanged = true;
            break;
        case 't':
            _seekmode = '0';
            pmLogging.LogLn("Seek stopped");
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

    currentFrequency = _radio->getCurrentFrequency();
    if(previousFrequency != currentFrequency && now > frequencychangetimeout) 
    {
        pmLogging.LogLn("Frequency changed to " + String(currentFrequency) );
        previousFrequency = currentFrequency;
        
        if(TunerButtons.SavePresetButtonPressed)
            SaveCurrentChannel(_current_station_preset);

        RDSMessage.clear();
        RDSStationName.clear();
        RDSTime.clear();
        delay(200);
        _lastRDSUpdate = now;
    }

    if(_bands[_currentBand].bandType == FM_BAND_TYPE)
    {
        if(now - _lastRDSUpdate > 50)
        {
            checkRDS();
            _lastRDSUpdate = now;
        }
    }
}