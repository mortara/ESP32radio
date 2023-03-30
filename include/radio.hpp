#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Preferences.h>
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "speaker.hpp"
#include "fmtuner.hpp"
#include "internetradio.hpp"
#include "bluetoothplayer.hpp"
#include "channelswitch.hpp"
#include "vs1053.hpp"
#include "tunerbuttons.hpp"
#include "wifimanager.hpp"
#include "preselect_leds.hpp"
#include "preselect_buttons.hpp"

class Radio
{

    private:
        TwoWire *_i2cwire;
        WIFIManager *wifi;
        Speaker *_spk;
        VS1053Player *_player;
        ChannelSwitch *_channel;
        FMTuner *_fmtuner;
        InternetRadio *_inetRadio;
        BlueToothPlayer *_bluetoothplayer;
        TunerButtons *_tunerbuttons;
        PreselectLeds *_preselectLeds;
        PreselectButtons *_preselectButtons;
        
        int _currentInput = 0;
        
    public:
        Radio();
        void Loop();
        void ExecuteCommand(char ch);
        void SwitchInput(int newinput);

};