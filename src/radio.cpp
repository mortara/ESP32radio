#include "radio.hpp"
#include "webserver_callbacks.hpp"

void Task1code(void *parameter) 
{
    pmCommonLib.Start();

    for(;;) {
      delay(50);
      _radio.SecondaryLoop();
    }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    String topicstr = String(topic);    
    String msg;
    for (byte i = 0; i < length; i++) {
        char tmp = char(payload[i]);
        msg += tmp;
    }

    pmLogging.LogLn(topicstr + ": " + msg);

    if(topicstr == "homeassistant/select/ESP32Radio_Radio/Preset")
    {
        _radio.ExecuteCommand(msg[0]);
    }

    if(topicstr == "homeassistant/select/ESP32Radio_Internetradio/Country")
    {
        _radio._inetRadio->SetSeekCountry(msg);
    }

    if(topicstr == "homeassistant/select/ESP32Radio_Internetradio/Category")
    {
        _radio._inetRadio->SetSeekCategory(msg);
    }

    if(topicstr == "homeassistant/select/ESP32Radio_Internetradio/SeekStation")
    {
        _radio._inetRadio->SetStation(msg);
    }

    if(topicstr == "homeassistant/button/ESP32Radio_SI4735/StartSeek" && msg == "PRESS")
    {

        _radio.ExecuteCommand('u');
    }

    if(topicstr == "homeassistant/button/ESP32Radio_SI4735/StopSeek" && msg == "PRESS")
    {

        _radio.ExecuteCommand('t');
    }
}

void Radio::Setup()
{
    /*
        I2C Adresses:

        Wire 1
            0x20    ClockButtons
            0x11    Si4735
            0x27    ClockDisplay
            0x40    INA219
            0x77    BMP180

        Wire 2
            0x21    PreselectLeds
            0x22    TunerButtons
            0x23    PreselectButtons
            0x24    ChannelSwicth relais
            0x25    ChannelButtons
    
    */

    Wire.begin(42,41, 10000);

    _clockButtons = new ClockButtons(&Wire, 0x20);
    delay(100);

    if(_clockButtons->Loop() == 8)
    {
        OTAOnly = true;
        return;
    }

    _spk = new Speaker(21);
    
    //Wire.setClock(10000);
    _freq_display = new FrequencyDisplay();
    ClockDisplay.StartUp(0x27);
    ClockDisplay.DisplayText("Starte I2C Bus 2 ...",0);
    
    _i2cwire = new TwoWire(1);
    _i2cwire->begin(39,40);
    
    _preselectLeds  = new PreselectLeds(_i2cwire, 0x21);
    _preselectLeds->SetLed(0);
    
    ClockDisplay.DisplayText("Starte RTC ...",0);
    _clock = new Clock(Wire);
    
    ClockDisplay.DisplayText("Starte Kanalrelais ...",0);
    _channel = new ChannelSwitch(_i2cwire, 0x24);

    FrequencyIndicator.Setup(26, 8800,10800, 8800);
    SignalIndicator.Setup(25, 0,5, 0);
    
    ClockDisplay.DisplayText("Starte VS1053 ...",0);
    MP3Player.Setup();
    
    ClockDisplay.DisplayText("Starte Si4735 ...",0);
    _fmtuner = new FMTuner4735();
    
    ClockDisplay.DisplayText("Starte Internetradio ...",0);
    _inetRadio = new InternetRadio();

    /*_clockDisplay->DisplayText("Starte Bluetooth ...",0);
    _bluetoothplayer = new BlueToothPlayer();*/
    
    ClockDisplay.DisplayText("Starte Frontelemente ...",0);
    TunerButtons.Setup(_i2cwire, 0x22);
    _preselectButtons = new PreselectButtons(_i2cwire, 0x23);
    _channelButtons = new ChannelButtons(_i2cwire, 0x25);
    
    RotaryEncoder.Setup(45,48,14);
    
    ClockDisplay.DisplayText("Starte Sensoren ...",0);
    TemperatureSensor1.Begin(0x77);
    PowerSensor.Begin(0x40);
    
    ClockDisplay.DisplayText("Fertig!",0);

    _lastMQTTUpdate = _lastClockUpdate = millis();

    pwmFan1.Begin(10);

    Serial.println("Starting secondary features task...");
    xTaskCreatePinnedToCore(
    Task1code, /* Function to implement the task */
    "Task1", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &Task1,  /* Task handle. */
    0); /* Core where the task should run */
    
    pmLogging.LogLn("Radio setup complete!");
    setupdone = true;
}



void Radio::ShowPercentage(int value, int max)
{
    _preselectLeds->ShowPercentage(value, max);
}

void Radio::Stop()
{
    _spk->TurnOff();
}

void Radio::Start()
{
    _spk->TurnOn();
}

void Radio::ExecuteCommand(char ch)
{
    // s -> Toggle speaker on/off
    // t -> Test VS1053 with sine signal
    // + -> Raise VS1053/Si4703 volume
    // - -> Lower VS1053/Si4703 volume
    // a,b,c,d -> Switch input channel
    // 1,2,3,4,5,6,7,8 -> Select preset channel
    // q -> Restart ESP
    // w -> Display WIFI status
    // y -> Display DAC Status
    // v -> Reconnect WIFI
    // r -> Display RDS data
    // u -> FMTuner seek up
    // i -> FMTuner step down
    // z -> FMTuner seek down
    // o -> FMTuner step up
    // t -> Stop Tuner seek
    // x -> FMTuner toggle save mode
    // n -> Display current Time of RTC
    // j -> Display Powersensor info

    pmLogging.LogLn("Executing keypress command: " + String(ch));

    switch(ch)
    {
        case 'a':
            SwitchInput(INPUT_LW);
            break;
        case 'b':
            SwitchInput(INPUT_MW);
            break;
        case 'c':
            SwitchInput(INPUT_SW1);
            break;
        case 'd':
            SwitchInput(INPUT_SW2);
            break;
        case 'e':
            SwitchInput(INPUT_SW3);
            break;
        case 'f':
            SwitchInput(INPUT_FM);
            break;
        case 'g':
            SwitchInput(INPUT_AUX);
            break;
        case 'h':
            SwitchInput(INPUT_INET);
            break;
        case 'j':
            FrequencyIndicator.DisplayInfo();
            SignalIndicator.DisplayInfo();
            PowerSensor.DisplayInfo();
            TemperatureSensor1.DisplayInfo();
            break;
        case 'J':    
            _clockButtons->DisplayInfo();
            _clock->DisplayInfo();
            _fmtuner->DisplayInfo();
            _inetRadio->DisplayInfo();
            MP3Player.DisplayInfo();
            break;
        case 'n':
            _clock->SetByNTP();
            break;
        case 'p':
            _powersavemode++;
            EnterPowerSaveMode(_powersavemode);
            break;
        case 'P':
            _powersavemode--;
            EnterPowerSaveMode(_powersavemode);
            break;
        case 'q':
            ESP.restart();
            break;
        case 's':
            _spk->Toggle();
            break;
        case 'v':
            pmCommonLib.WiFiManager.Connect();
            break;
        case 'V':
            pmCommonLib.WiFiManager.Disconnect();
            break;
        case 'w':
            pmCommonLib.WiFiManager.DisplayInfo();
            break;
        case 'x':
            pwmFan1.StartFan();
            break;
        case 'X':
            pwmFan1.StopFan();
            break;
        case 'u':
            _fmtuner->Loop('u');
            break;
        case 't':
            _fmtuner->Loop('t');
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            if(_currentPlayer != PLAYER_EXT)
            {
                int preset = ch - '1';
                pmLogging.LogLn("Switch to preset: " + String(ch));
                _preselectLeds->SetLed(preset);
                if(_currentOutput == OUTPUT_SI47XX)
                    _fmtuner->SwitchPreset(preset);
                else if(_currentInput == INPUT_INET)
                    _inetRadio->SwitchPreset(preset);
                
                _currentPreset = preset;
            }
            
            break;
    }
}

void Radio::setupMQTT()
{
    if(mqttsetup)
        return;

    pmLogging.LogLn("Setting up MQTT client");

    if(!pmCommonLib.MQTTConnector.SetupSensor("Input","Radio", "", ""))
    {
        pmLogging.LogLn("Unable to setup MQTT client");
        return;
    }

    pmCommonLib.MQTTConnector.SetupSelect("Preset", "Radio", "", "mdi:radio", std::vector<String>({"1","2","3","4","5","6","7","8"}));
    pmCommonLib.MQTTConnector.SetupSensor("DateTime", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("FreeHeap", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("FreePSRAM", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("FreeSketchSpace", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("ChipCores", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("CPUFreqpCores", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("ChipModel", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("FlashSize", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("FlashMode", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("FrequencyDisplay", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("ClockDisplay1", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("ClockDisplay2", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("UpTime",  "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("LoopTime", "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("PowerSaveMode",  "Radio", "", "");
    pmCommonLib.MQTTConnector.SetupSensor("FanRunning", "Radio", "", "");
    pmLogging.LogLn("mqtt setup done!");

    mqttsetup = true;
}

void Radio::EnterPowerSaveMode(int lvl)
{
    pmLogging.LogLn("Entering powersaving mode " + String(lvl));

    switch(lvl)
    {
        case 0:  // No power saving
            _freq_display->TurnOnOff(true);
            ClockDisplay.TurnOnOff(true);
            break;
        case 1: // minimal power saving
            _freq_display->TurnOnOff(true);
            ClockDisplay.TurnOnOff(false);
            break;
        case 2: // maximal power saving
            _freq_display->TurnOnOff(false);
            ClockDisplay.TurnOnOff(false);
            _preselectLeds->SetLed(99);
            break;
        default: // all other value lead to immediate shutdown
            _freq_display->TurnOnOff(false);
            ClockDisplay.TurnOnOff(false);
            this->Stop();
            break;

    }

    _powersavemode = lvl;
}

char Radio::Loop()
{
    if(!setupdone || OTAOnly)
        return ' '; 

    RotaryEncoder.Loop();
    
    char ch = ' ';
    if (Serial.available())
        ch = Serial.read();

    
    if(ch == ' ')
        ch = pmCommonLib.WebSerial.GetInput();
    
        
    int cha_btn = _channelButtons->Loop();
    
    if(cha_btn != 0)
    {
        pmLogging.LogLn("Channel button pressed: " + String(cha_btn));
        switch(cha_btn)
        {
            case 1: ch = 'a'; break;  // lw
            case 2: ch = 'b'; break;  // mw
            case 3: ch = 'c'; break;  // sw1
            case 4: ch = 'd'; break;  // sw2
            case 5: ch = 'e'; break;  // sw3
            case 6: ch = 'f'; break;  // fm
            case 7: ch = 'g'; break;  // phono (aux)
            case 8: ch = 'h'; break;  // tape1 (internet)
        }
    }
    
    int pre_btn = _preselectButtons->Loop();
    if(pre_btn != 0)
    {
        pmLogging.LogLn("Preselect button pressed: " + String(pre_btn));
        switch(pre_btn)
        {
            case 1: ch = '1'; break;
            case 2: ch = '2'; break;
            case 4: ch = '3'; break;
            case 8: ch = '4'; break;
            case 16: ch = '5'; break;
            case 32: ch = '6'; break;
            case 64: ch = '7'; break;
            case 128: ch = '8'; break;
        }
        
    }
    
    int btn = TunerButtons.Loop();
    if(btn != 0)
    {
        pmLogging.LogLn("Tunerbutton pressed: " + String(btn));
        switch(btn)
        {
            case 16: ch = 't'; break; // Stop seek
            case 8:  ch = 'z'; break; // seekdown
            case 32: ch = 'u'; break; // seekdown
            case 64: ch = 'I'; break; // autotune up
            case 4:  ch = 'O'; break; // autotune down
            case 2:  ch = 'i'; break; // step down
            case 128:ch = 'o'; break; // step up
        }
    }

    int clockbtns = _clockButtons->Loop();
    if(clockbtns != 0)
    {

    }

    if(ch != ' ')
    {
        ExecuteCommand(ch);
    }
    
    if(_currentPlayer == PLAYER_SI47XX)
    {
        _fmtuner->Loop(ch);
        
    } else if(_currentPlayer == PLAYER_WEBRADIO)
    {
        MP3Player.ExecuteCommand(ch);
        _inetRadio->Loop(ch);
        
    } /*else if(_currentPlayer == PLAYER_BT)
    {
        MP3Player.ExecuteCommand(ch);
        _bluetoothplayer->Loop(ch);
    }*/
    
    unsigned long now = millis();
    if(now - _lastDisplayUpdate > 100)
    {
        _lastDisplayUpdate = now;
        int freqfront = 0;
        if(_currentPlayer == PLAYER_SI47XX)
        {
            _clockDisplayText0 = _fmtuner->GetClockDisplayText();
            _frequencyDisplayText = _fmtuner->GetFreqDisplayText();
            freqfront = 1;
        } else if(_currentPlayer == PLAYER_WEBRADIO)
        {
            _clockDisplayText0 = _inetRadio->GetClockDisplayText();
            _frequencyDisplayText = _inetRadio->GetFreqDisplayText();           
        } else if(_currentPlayer == PLAYER_EXT)
        {
            _clockDisplayText0 = AuxPlayer.GetClockDisplayText();
            _frequencyDisplayText = AuxPlayer.GetFreqDisplayText();
        }

        ClockDisplay.DisplayText(_clockDisplayText0, 0);
        _freq_display->DisplayText(_frequencyDisplayText, freqfront);

        _freq_display->Loop();
        ClockDisplay.Loop();
    }

    delay(10);

    return ch;
}

void Radio::SecondaryLoop()
{
    if(!setupdone || OTAOnly)
        return; 

    unsigned long now = millis();

    pmCommonLib.Loop();

    if(now - _lastClockUpdate > 1000)
    {
        _lastClockUpdate = millis();
        TemperatureSensor1.Loop();
        PowerSensor.Loop();
       
        if(TemperatureSensor1.GetLastTemperatureReading() > 45 && pwmFan1.FanState == false)
            pwmFan1.StartFan();

        if(TemperatureSensor1.GetLastTemperatureReading() < 40 && pwmFan1.FanState == true)
            pwmFan1.StopFan();

        _clockDisplayText1 = _clock->GetDateTimeString(false);
        ClockDisplay.DisplayText(_clockDisplayText1, 1);

        if(WiFi.isConnected() && pmCommonLib.MQTTConnector.isActive() && !mqttsetup)
            setupMQTT();

        if(mqttsetup && (now - _lastMQTTUpdate > 5000UL))
        {
           
            JsonDocument payload;      
            payload["Input"] = String(_currentInput);
            payload["DateTime"] = _clock->GetDateTimeString();
            payload["FreeHeap"] = String(esp_get_free_heap_size());
            payload["FreePSRAM"] = String(ESP.getFreePsram());
            payload["FreeSketchSpace"] = String(ESP.getFreeSketchSpace());
            payload["ChipCores"] = String(ESP.getChipCores());
            payload["CPUFreqpCores"] = String(ESP.getCpuFreqMHz());
            payload["ChipModel"] = String(ESP.getChipModel());
            payload["FlashSize"] = String(ESP.getFlashChipSize());
            payload["FlashMode"] = ""; // String(ESP.getFlashChipMode());
            payload["FrequencyDisplay"] = _frequencyDisplayText;
            payload["ClockDisplay1"] = _clockDisplayText0;
            payload["ClockDisplay2"] = _clockDisplayText1;
            payload["UpTime"] =  uptime_formatter::getUptime();
            payload["LoopTime"] =  String(LoopTime);
            payload["PowerSaveMode"] = String(_powersavemode);
            payload["FanRunning"] = String(pwmFan1.FanState);
           
            pmCommonLib.MQTTConnector.PublishMessage(payload, "Radio", true);

            JsonDocument payload2; 
            payload2["Preset"] = String(_currentPreset + 1);
            pmCommonLib.MQTTConnector.PublishMessage(payload2, "Radio", true, "", SELECT);

            _lastMQTTUpdate = now;

        }
    }
}

void Radio::SwitchInput(uint8_t newinput)
{
    if(newinput == _currentInput)
        return;
        
    pmLogging.LogLn("Switch Input to  " + String(newinput));

    uint8_t new_output = _currentOutput;
    uint8_t new_player = _currentPlayer;

    switch(newinput)
    {
        case INPUT_FM:
        case INPUT_MW:
        case INPUT_SW1:
        case INPUT_SW2:
        case INPUT_SW3:
        case INPUT_LW:
            new_output = OUTPUT_SI47XX;
            new_player = PLAYER_SI47XX;
            break;
        case INPUT_AUX:
            new_output = OUTPUT_AUX;
            new_player = PLAYER_EXT;
            break;
        case INPUT_BT:
            new_player = PLAYER_BT;
            new_output = OUTPUT_VS1053;
        case INPUT_INET:
            new_player = PLAYER_WEBRADIO;
            new_output = OUTPUT_VS1053;
            break;
    }

    if(new_player != _currentPlayer)
    {
        pmLogging.LogLn("Stopping old player");
        // Stopping the currently running player
        if(_currentPlayer == PLAYER_SI47XX)
            _fmtuner->Stop();
        //else if(_currentPlayer == PLAYER_BT)
        //    _bluetoothplayer->Stop();
        else if(_currentPlayer == PLAYER_WEBRADIO)
            _inetRadio->Stop();
        
        pmLogging.LogLn("Starting new player");
        // Starting the new player
        if(new_player == PLAYER_SI47XX)
        {
            _currentPreset = _fmtuner->Start(newinput - 1);
            _preselectLeds->SetLed(_currentPreset);
        }
        //else if(new_player == PLAYER_BT)
        //    _bluetoothplayer->Start();
        else if(new_player == PLAYER_WEBRADIO)
        {
            if(!pmCommonLib.WiFiManager.IsConnected())
                if(!pmCommonLib.WiFiManager.Connect())
                {
                    pmLogging.LogLn("Could not connect to WIFI network!");
                }
                    
            _currentPreset = _inetRadio->Start();
            _preselectLeds->SetLed(_currentPreset);
        } else if(new_player == PLAYER_EXT)
        {
            _preselectLeds->SetLed(99);
        }
    }

    if(new_player == PLAYER_SI47XX)
    {
        _fmtuner->SwitchBand(newinput - 1);
    }

    if(new_output != _currentOutput)
    {
        _spk->TurnOff();
        _channel->SetChannel(new_output);
        _currentOutput = new_output;
        _spk->TurnOn();
    }

    _currentPlayer = new_player;
    _currentInput = newinput;
    
    pmLogging.LogLn("Input switch done!");
}

Radio _radio;