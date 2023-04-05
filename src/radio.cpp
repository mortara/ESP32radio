#include "radio.hpp"

Radio::Radio()
{
    _spk = new Speaker(27);

    _i2cwire = new TwoWire(1);
    _i2cwire->begin(33,32, 10000);
    _preselectLeds  = new PreselectLeds(_i2cwire, 0x21);

    // Initialize SPIFFS
    if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
    }

    wifi = new WIFIManager();
    _clock = new Clock(_i2cwire);
    _channel = new ChannelSwitch(_i2cwire, 0x20);
    _player = new VS1053Player();
    _fmtuner = new FMTuner4735();
    _inetRadio = new InternetRadio(_player);
    _bluetoothplayer = new BlueToothPlayer(_player);
    _tunerbuttons = new TunerButtons(_i2cwire, 0x22);
    _preselectButtons = new PreselectButtons(_i2cwire, 0x24);
    _channelButtons = new ChannelButtons(_i2cwire, 0x25);
    
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

    if(_currentInput == INPUT_FM)
    {
        _fmtuner->SetSaveMode(_tunerbuttons->SavePresetButtonPressed);
        _fmtuner->Loop(ch);
    } else if(_currentInput == INPUT_INET)
    {
        _player->ExecuteCommand(ch);
        _inetRadio->Loop(ch);
    } else if(_currentInput == 3)
    {
        _player->ExecuteCommand(ch);
        _bluetoothplayer->Loop(ch);
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
        // Stopping the currently running player
        if(_currentPlayer == PLAYER_SI47XX)
            _fmtuner->Stop();

        if(_currentPlayer == PLAYER_BT)
            _bluetoothplayer->Stop();

        if(_currentPlayer == PLAYER_WEBRADIO)
            _inetRadio->Stop();
        
        // Starting the new player
        if(new_player == PLAYER_SI47XX)
            _fmtuner->Start();

        if(new_player == PLAYER_BT)
            _bluetoothplayer->Start();

        if(new_player == PLAYER_WEBRADIO)
        {
            if(!wifi->IsConnected())
                wifi->Connect();

            _inetRadio->Start();
        }
    }

    if(new_player == PLAYER_SI47XX)
        _fmtuner->SwitchBand(newinput);

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
    
}
