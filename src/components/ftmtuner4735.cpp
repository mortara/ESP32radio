#include "fmtuner4735.hpp"
#include "mqtt.hpp"
#include "rotary_encoder.hpp"

FMTuner4735::FMTuner4735()
{
    WebSerialLogger.println("Start Si4735 ...");

    digitalWrite(SI7435_RESET_PIN, HIGH);
    //Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL, 10000);

    
    _radio = new SI4735();
    uint8_t adr = _radio->getDeviceI2CAddress(SI7435_RESET_PIN);
    if(adr == 0)
    {
        WebSerialLogger.println("Device address: " + String(adr));
        WebSerialLogger.println("Si4735 not found!" );
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
    _lastRotaryRead = frequencychangetimeout = _lastRDSUpdate = _lastUpdate = millis();
}

void FMTuner4735::setupMQTT()
{
    if(mqttsetup)
        return;

    WebSerialLogger.println("Setting up si4735 MQTT client");

    MQTTConnector.SetupSensor("Frequency", "sensor", "SI4735", "frequency", "Hz", "mdi:sine-wave");
    MQTTConnector.SetupSensor("Band", "sensor", "SI4735", "", "", "mdi:radio");
    MQTTConnector.SetupSensor("BandLowerLimit", "sensor", "SI4735", "frequency", "Hz", "mdi:sine-wave");
    MQTTConnector.SetupSensor("BandHighLimit", "sensor", "SI4735", "frequency", "Hz", "mdi:sine-wave");
    MQTTConnector.SetupSensor("RSSI", "sensor", "SI4735", "signal_strength", "dB", "mdi:radio-tower");
    MQTTConnector.SetupSensor("SNR", "sensor", "SI4735", "signal_strength", "dB", "mdi:radio-tower");
    MQTTConnector.SetupSensor("RDSMSG", "sensor", "SI4735", "", "", "mdi:message-processing");
    MQTTConnector.SetupSensor("BandData", "sensor", "SI4735", "", "", "mdi:radio-tower");
    WebSerialLogger.println("si4735 Sensor mqtt setup done!");

    mqttsetup = true;
}

void FMTuner4735::sendMQTTState()
{
    Band b = _bands[_currentBand];

    DynamicJsonDocument payload(2048);
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
        sprintf(str,"Step %2.2d | Bw %2.2d | disableAgc %2.2d  | agcIdx %2.2d | agcNdx %2.2d | avcIdx %2.2d", tabFmStep[b.currentStepIdx], bandwidthFM[b.bandwidthIdx].idx, b.disableAgc, b.agcIdx, b.agcNdx, b.avcIdx );
    else
        sprintf(str,"Step %2.2d | Bw %2.2d | disableAgc %2.2d  | agcIdx %2.2d | agcNdx %2.2d | avcIdx %2.2d", tabAmStep[b.currentStepIdx], bandwidthAM[b.bandwidthIdx].idx, b.disableAgc, b.agcIdx, b.agcNdx, b.avcIdx );

    payload["BandData"] = String(str);

    String state_payload  = "";
    serializeJson(payload, state_payload);
    
    MQTTConnector.PublishMessage(state_payload, "SI4735");
}

void FMTuner4735::Start(uint8_t band, uint8_t preset)
{
    if(!_active)
        return;

    WebSerialLogger.println("FMTuner start ...");

    
    SignalIndicator.SetRange(0, 127);
    
    SwitchBand(band);
    _radio->setVolume(_volume);
    
    SwitchPreset(preset);

    DisplayInfo();
}

void FMTuner4735::Stop()
{
    WebSerialLogger.println("FMTuner stop ...");
}

void FMTuner4735::SwitchBand(uint8_t bandIdx)
{
    if(!_active)
        return;

    WebSerialLogger.println("Switch to band " + String(bandIdx));
    if(bandIdx == _currentBand)
    {
        WebSerialLogger.println("Band already active!");
        return;
    }

    Band b = _bands[bandIdx];

    if(_currentBand != -1)
    {
        SavePresets();
    }

    if (b.bandType == FM_BAND_TYPE)
    {
        WebSerialLogger.println("... FM Mode band " + String(b.bandName));
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
        WebSerialLogger.println("... AM Mode band " + String(b.bandName));
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
    delay(10);

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


    Band b = _bands[_currentBand];
    uint16_t fwidth = b.maximumFreq - b.minimumFreq;

    if(freq < b.minimumFreq)
        freq += fwidth;

    if(freq > b.maximumFreq)
        freq -= fwidth;

    _radio->setFrequency(freq);
    currentFrequency = _radio->getCurrentFrequency();
    FrequencyIndicator.SetValue(currentFrequency);
    return currentFrequency;
}

void FMTuner4735::DisplayInfo()
{
    if(!_active)
        return;

    WebSerialLogger.println("Radio info:");
    _radio->getStatus();
    _radio->getCurrentReceivedSignalQuality();
    WebSerialLogger.print("You are tuned on ");
    if (_radio->isCurrentTuneFM())
    {
        WebSerialLogger.print(String(currentFrequency / 100.0, 2));
        WebSerialLogger.print("MHz ");
        WebSerialLogger.print((_radio->getCurrentPilot()) ? "STEREO" : "MONO");
    }
    else
    {
        WebSerialLogger.print(String(currentFrequency));
        WebSerialLogger.print("kHz");
    }
    WebSerialLogger.print(" [SNR:");
    WebSerialLogger.print(String(_radio->getCurrentSNR()));
    WebSerialLogger.print("dB");

    WebSerialLogger.print(" Signal:");
    WebSerialLogger.print(String(_radio->getCurrentRSSI()));
    WebSerialLogger.println("dBuV]");

    uint8_t offset = _radio->getCurrentSignedFrequencyOffset();
    WebSerialLogger.println(" SignedFrequencyOffset: " + String(offset) + " kHz");

    WebSerialLogger.println("Volume: " + String(_volume));
    
    if(_savemode)
        WebSerialLogger.println("Preset save mode is: on");
    else
        WebSerialLogger.println("Preset save mode is: off");

    if(rdsMsg != NULL && rdsMsg[0] != '\0')
        WebSerialLogger.println("RDS Msg: " + String(rdsMsg));

    if(stationName != NULL && stationName[0] != '\0')    
        WebSerialLogger.println("RDS Station: " + String(stationName));

    if(rdsTime != NULL && rdsTime[0] != '\0')
        WebSerialLogger.println("RDS Time" + String(rdsTime));
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
    if(!_active)
    {
        return "Tuner not found!";
    }

    if(clockdisplaypage == 1)
    {
        return RDSMessage;
    }


    uint8_t rssi = _radio->getCurrentRSSI();
    String t = GetFreqDisplayText() + "  RSSI: " + String(rssi) + " ";
    if(t.length() > 16)
        t = t.substring(0,16);

    return t;
}

void FMTuner4735::SetSaveMode(bool onoff)
{
    _savemode = onoff;
}

void FMTuner4735::SwitchPreset(uint8_t num)
{
    WebSerialLogger.println("FMTuner::SwitchPreset to " + String(num));
    _current_station_preset = num;
    currentFrequency = setFrequency(_station_presets[num]);
}

void FMTuner4735::SaveCurrentChannel(uint8_t preset)
{
    WebSerialLogger.println("Saving frequency " + String(currentFrequency)+ " as preset " + preset);
    _station_presets[preset] = currentFrequency;

    SavePresets();
}

void FMTuner4735::LoadPresets()
{
    WebSerialLogger.println("Load presets for band " + String(_currentBand));
    if(_currentBand < 0 || _currentBand > 7)
    {
        WebSerialLogger.println(" ... invalid band");
        return; 
    }    

    Band b = _bands[_currentBand];
    Preferences _prefs;
    String prefname = "radio_tuner_" + String(b.bandName);
    if(!_prefs.begin(prefname.c_str(), false)) 
    {
        WebSerialLogger.println("unable to open preferences");
    }
    else
    {
        WebSerialLogger.println("Loading presets from SPIFFS");
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
    WebSerialLogger.println("Save presets band " + String(_currentBand));
    if(_currentBand < 0 || _currentBand > 7)
    {
        WebSerialLogger.println(" ... invalid band");
        return; 
    } 

    Band b = _bands[_currentBand];

    Preferences _prefs;
    String prefname = "radio_tuner_" + String(b.bandName);
    WebSerialLogger.println(prefname);
    if(!_prefs.begin(prefname.c_str(), false))
    {
        WebSerialLogger.println("Could not open preset file " + prefname);
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
                //WebSerialLogger.println("RDS Message: '" + RDSMessage + "'");
            }

            String stationname = UpdateRDSMessage(RDSStationName, stationName);
            if(stationname != RDSStationName && stationname != "")
            {
                RDSStationName = stationname;
                //WebSerialLogger.println("RDS Stationname: '" + RDSStationName+ "'");
            }

            String time = UpdateRDSMessage(RDSTime, rdsTime);
            if(time != RDSTime && time != "")
            {
                RDSTime = time;
                //WebSerialLogger.println("RDS Time: '" + RDSTime+ "'");
            }
        }
    }
}

void FMTuner4735::Loop(char ch)
{
    if(!_active)
        return;
    unsigned long now = millis();

    if((now - clockdisplaypagetimer) > 15000)
    {
        clockdisplaypage++;
        if(clockdisplaypage == 2)
            clockdisplaypage = 0;
        
        clockdisplaypagetimer = now;

        if(mqttsetup)
        {   
            sendMQTTState();
        } else if(MQTTConnector.isActive())
            setupMQTT();
    }

    if(now - _lastUpdate > 500)
    {
        _radio->getCurrentReceivedSignalQuality();
        SignalIndicator.SetValue(_radio->getCurrentRSSI());
        _lastUpdate = now;
    }

    if(_seekmode != '0' && ch != 't')
    {
        _radio->getCurrentReceivedSignalQuality();
        uint8_t rssi = _radio->getCurrentRSSI();
        uint8_t snr = _radio->getCurrentSNR();
        if(rssi < 24 || (now - _seektimer) > 4000)
        {
            ch = _seekmode;
            char str[100];
            sprintf(str,"Seeking .... Freq: %2.2d | RSSI: %2.2d | SNR: %2.2d", currentFrequency, rssi, snr);
            WebSerialLogger.println(str);
        }

    }

    if(now - _lastRotaryRead > 200)
    {
        _lastRotaryRead = now;
        int rotary = RotaryEncoder.GetCounter();
        if(rotary != _lastRotaryCount)
        {
            int diff = (rotary - _lastRotaryCount) / 5;
            if(diff < 1)
                diff = 1;

            _lastRotaryCount = rotary;
            _seekmode = '0';
            setFrequency(currentFrequency + _smallstep * diff);
            frequencychangetimeout = now + 1000;
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
            WebSerialLogger.println("Frequency up " + String(_smallstep));
            break;
        case 'i':
            setFrequency(currentFrequency - _smallstep);
            _seekmode = '0';
            channelchanged = true;
            WebSerialLogger.println("Frequency down " + String(_smallstep));
            break;
        case 'I':
            setFrequency(currentFrequency + _step);     
            _seekmode = '0';
            channelchanged = true;
            WebSerialLogger.println("Frequency up " + String(_step));
            break;
        case 'O':
            setFrequency(currentFrequency - _step);
            _seekmode = '0';
            channelchanged = true;
            WebSerialLogger.println("Frequency down " + String(_step));
            break;
        case 'u':
            _seekmode = ch;
            _radio->frequencyUp();
            _seektimer = millis();
            channelchanged = true;
            WebSerialLogger.println("Start seek up");
            break;
        case 'z':
            _seekmode = ch;
            _radio->frequencyDown();
            WebSerialLogger.println("Start seek down");
            _seektimer = millis();
            channelchanged = true;
            break;
        case 't':
            _seekmode = '0';
            WebSerialLogger.println("Seek stopped");
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

    currentFrequency = _radio->getCurrentFrequency();
    if(previousFrequency != currentFrequency && now > frequencychangetimeout) 
    {
        WebSerialLogger.println("Frequency changed!");
        previousFrequency = currentFrequency;
        
        if(_savemode)
            SaveCurrentChannel(_current_station_preset);

        RDSMessage.clear();
        RDSStationName.clear();
        RDSTime.clear();
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