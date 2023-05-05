#include "radio.hpp"

Radio::Radio()
{
    _spk = new Speaker(27);
    Wire.begin();
    _freq_display = new FrequencyDisplay();
    _clockDisplay = new ClockDisplay();
    _clockDisplay->DisplayText("Starte I2C Bus 2 ...",0);

    _i2cwire = new TwoWire(1);
    _i2cwire->begin(33,32, 10000);

    
    _preselectLeds  = new PreselectLeds(_i2cwire, 0x21);
    _preselectLeds->SetLed(0);

    _clockDisplay->DisplayText("Starte WIFI ...",0);
    wifi = new WIFIManager();

    _clockDisplay->DisplayText("Starte MQTT ...",0);
    _mqttConnector = new MQTTConnector();
    
    _clockDisplay->DisplayText("Starte RTC ...",0);
    _clock = new Clock(_i2cwire);

    _clockDisplay->DisplayText("Starte Kanalrelais ...",0);
    _channel = new ChannelSwitch(_i2cwire, 0x24);

    _clockDisplay->DisplayText("Starte VS1053 ...",0);
    _player = new VS1053Player();
    _pwm_indicator_freq = new DACIndicator(26, 8800,10800, 8800);
    _pwm_indicator_signal = new DACIndicator(25, 0,5, 0);

    _clockDisplay->DisplayText("Starte Si4735 ...",0);
    _fmtuner = new FMTuner4735(_pwm_indicator_freq, _pwm_indicator_signal, _mqttConnector);

    _clockDisplay->DisplayText("Starte Internetradio ...",0);
    _inetRadio = new InternetRadio(_player, _pwm_indicator_freq, _pwm_indicator_signal);

    _clockDisplay->DisplayText("Starte Bluetooth ...",0);
    _bluetoothplayer = new BlueToothPlayer(_player);

    _clockDisplay->DisplayText("Starte Frontelemente ...",0);
    _tunerbuttons = new TunerButtons(_i2cwire, 0x22);
    _preselectButtons = new PreselectButtons(_i2cwire, 0x23);
    _channelButtons = new ChannelButtons(_i2cwire, 0x25);
    
    _rotary1 = new RotaryEncoder(34,35,39);
    _lastClockUpdate = millis();

    

    _clockDisplay->DisplayText("Starte Temperatursensor ...",0);
    _tempSensor1 = new TemperatureSensor(_mqttConnector);

    //wifi->Connect();
    
    _clockDisplay->DisplayText("Fertig!",0);
    Serial.println("Radio setup complete!");
}

void Radio::ExecuteCommand(char ch)
{
    if (ch == 'a') 
    {
        SwitchInput(INPUT_LW);
    } else if (ch == 'b') 
    {
        SwitchInput(INPUT_MW);
    } else if (ch == 'c') 
    {
        SwitchInput(INPUT_SW1);
    } else if (ch == 'd') 
    {
        SwitchInput(INPUT_SW2);
    } else if (ch == 'e') 
    {
        SwitchInput(INPUT_SW3);
    } else if (ch == 'f') 
    {
        SwitchInput(INPUT_FM);
    } else if (ch == 'g') 
    {
        SwitchInput(INPUT_AUX);
    } else if (ch == 'h') 
    {
        SwitchInput(INPUT_INET);
    } else if (ch == 'q') 
    {
        ESP.restart();
    }else {

        int preset = ch - '0';
        if(preset > 0 && preset < 9)
        {
            Serial.println("Switch to preset: " + String(ch));
            _preselectLeds->SetLed(preset-1);
            if(_currentOutput == OUTPUT_SI47XX)
                _fmtuner->SwitchPreset(preset - 1);
            else if(_currentInput == INPUT_INET)
                _inetRadio->SwitchPreset(preset - 1);
        }
    }
}

void Radio::Loop()
{
    char ch = ' ';
    if (Serial.available())
    {
        ch = Serial.read();

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
        // x -> FMTuner toggle save mode
    }

    int cha_btn = _channelButtons->Loop();
    if(cha_btn != 0)
    {
        Serial.println("Channel button pressed: " + String(cha_btn));
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
        Serial.println("Preselect button pressed: " + String(pre_btn));
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

    int btn = _tunerbuttons->Loop();
    if(btn != 0)
    {
        Serial.println("Tunerbutton pressed: " + String(btn));
        switch(btn)
        {
            case 16: // ??
                ch = 't';
                break;
            case 8: // seekdown
                ch = 'z';
                break;
            case 32: // seekdown
                ch = 'u';
                break;
            case 64: // autotune up
                ch = 'I';
                break;
            case 4: // autotune down
                ch = 'O';
                break;
            case 2: // step down
                ch = 'i';
                break;
            case 128: // step up
                ch = 'o';
                break;
        }
    }

    if(ch != ' ')
    {
        ExecuteCommand(ch);
        _spk->ExecuteCommand(ch);
    }

    wifi->Loop(ch);
    if(!wifi->IsConnected() && millis() - wifi->LastConnectionTry() > 10000)
    if(!wifi->Connect())
    {
        Serial.println("Could not connect to WIFI network!");
    }

    if(_currentPlayer == PLAYER_SI47XX)
    {
        _fmtuner->SetSaveMode(_tunerbuttons->SavePresetButtonPressed);
        _fmtuner->Loop(ch);
        _freq_display->DisplayText(_fmtuner->GetFreqDisplayText(),1);
        _clockDisplay->DisplayText(_fmtuner->GetClockDisplayText(), 0);
    } else if(_currentPlayer == PLAYER_WEBRADIO)
    {
        _player->ExecuteCommand(ch);
        _inetRadio->Loop(ch);
        _clockDisplay->DisplayText(_inetRadio->GetClockDisplayText(), 0);
        _freq_display->DisplayText(_inetRadio->GetFreqDisplayText(),0);
    } else if(_currentPlayer == PLAYER_BT)
    {
        _player->ExecuteCommand(ch);
        _bluetoothplayer->Loop(ch);
    }
    _clock->Loop();
    _freq_display->Loop();
    _pwm_indicator_freq->Loop(ch);
    _pwm_indicator_signal->Loop(ch);
    _clockDisplay->Loop();
    _rotary1->Loop();
    _tempSensor1->Loop();
    _mqttConnector->Loop();
    
    uint16_t now = millis();
    if(now - _lastClockUpdate >= 1000)
    {
        _lastClockUpdate = now;
        char buf2[] = "DD.MM.YYYY hh:mm";
        _clockDisplay->DisplayText(_clock->Now().toString(buf2), 1);
    }
}

void Radio::SwitchInput(uint8_t newinput)
{
    if(newinput == _currentInput)
        return;
        
    Serial.println("Switch Input to  " + String(newinput));

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
        Serial.println("Stopping old player");
        // Stopping the currently running player
        if(_currentPlayer == PLAYER_SI47XX)
            _fmtuner->Stop();
        else if(_currentPlayer == PLAYER_BT)
            _bluetoothplayer->Stop();
        else if(_currentPlayer == PLAYER_WEBRADIO)
            _inetRadio->Stop();
        
        Serial.println("Starting new player");
        // Starting the new player
        if(new_player == PLAYER_SI47XX)
            _fmtuner->Start(newinput - 1);
        else if(new_player == PLAYER_BT)
            _bluetoothplayer->Start();
        else if(new_player == PLAYER_WEBRADIO)
        {
            if(!wifi->IsConnected())
                if(!wifi->Connect())
                {
                    Serial.println("Could not connect to WIFI network!");
                }
                    
            _inetRadio->Start();
        }
    }

    if(new_player == PLAYER_SI47XX)
        _fmtuner->SwitchBand(newinput - 1);

    if(new_output != _currentOutput)
    {
        _spk->TurnOff();
        _channel->SetChannel(new_output);
        _currentOutput = new_output;
        _spk->TurnOn();
    }

    _currentPlayer = new_player;
    _currentInput = newinput;
    delay(50);
    Serial.println("Input switch done!");
}
