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
    wifi->Start();

    _channel = new ChannelSwitch(_i2cwire, 0x20);
    _player = new VS1053Player();
    _fmtuner = new FMTuner();
    _inetRadio = new InternetRadio(_player);
    _bluetoothplayer = new BlueToothPlayer(_player);

    _tunerbuttons = new TunerButtons(_i2cwire, 0x22);
    
    _preselectButtons = new PreselectButtons(_i2cwire, 0x24);

    _currentInput = 0;

    Serial.println("Radio setup complete!");

    SwitchInput(1);
}

void Radio::ExecuteCommand(char ch)
{
    if (ch == 'a') 
    {
        SwitchInput(1);
    } else if (ch == 'b') 
    {
        SwitchInput(2);
    } else if (ch == 'c') 
    {
        SwitchInput(3);
    } else if (ch == 'd') 
    {
        SwitchInput(4);
    } else if (ch == 'e') 
    {
        SwitchInput(5);
    } else if (ch == 'q') 
    {
        ESP.restart();
    }else {

        int preset = ch - '0';
        if(preset > 0 && preset < 9)
        {
            Serial.println("Switch to preset: " + String(ch));
            _preselectLeds->SetLed(preset-1);
            if(_currentInput == 1)
                _fmtuner->SwitchPreset(preset - 1);
            else
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

    if(_currentInput == 1)
    {
        _fmtuner->SetSaveMode(_tunerbuttons->SavePresetButtonPressed);
        _fmtuner->Loop(ch);
    } else if(_currentInput == 2)
    {
        _player->ExecuteCommand(ch);
        _inetRadio->Loop(ch);
    } else if(_currentInput == 3)
    {
        _player->ExecuteCommand(ch);
        _bluetoothplayer->Loop(ch);
    }
}


void Radio::SwitchInput(int newinput)
{
    if(newinput == _currentInput)
        return;

    Serial.println("Switch Input to  " + String(newinput));

    _spk->TurnOff();
    _channel->TurnAllOff();

    if(_currentInput == 1)
        _fmtuner->Stop();

    if(_currentInput == 2)
        _inetRadio->Stop();

    if(_currentInput == 3)
        _bluetoothplayer->Stop();

    _currentInput = newinput;

    if(_currentInput == 1)
    {
        _fmtuner->Start();
        _channel->SetChannel(2);
    }

    if(_currentInput == 2)
    {
        _inetRadio->Start();
        _channel->SetChannel(1);
    }

    if(_currentInput == 3)
    {
        _bluetoothplayer->Start();
        _channel->SetChannel(1);
    }

    if(_currentInput == 4)
    {
        _channel->SetChannel(3);
    }

    if(_currentInput == 5)
    {
        _channel->SetChannel(4);
    }

    _spk->TurnOn();
}
