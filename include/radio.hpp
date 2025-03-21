#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Preferences.h>

#ifndef RADIO_H
#define RADIO_H

#include "pmCommonLib.hpp"
#include "clock.hpp"
#include "SPIFFS.h"
#include "speaker.hpp"
#include "fmtuner4735.hpp"
#include "internetradio.hpp"
//#include "bluetoothplayer.hpp"
#include "channelswitch.hpp"
#include "vs1053.hpp"
#include "tunerbuttons.hpp"
#include "preselect_leds.hpp"
#include "preselect_buttons.hpp"
#include "channelbuttons.hpp"
#include "frequency_display.hpp"
#include "clock_display.hpp"
#include "dacindicator.hpp"
#include "rotary_encoder.hpp"
#include "temperature_sensor.hpp"
#include "power_sensor.hpp"
#include "clock_buttons.hpp"
#include "uptime_formatter.h"
#include "auxplayer.hpp"
#include "menu.hpp"
#include "FanController.hpp"

#define INPUT_LW 1
#define INPUT_MW 2
#define INPUT_SW1 3
#define INPUT_SW2 4
#define INPUT_SW3 5
#define INPUT_FM 6
#define INPUT_AUX 7
#define INPUT_INET 8
#define INPUT_BT 9

#define PLAYER_SI47XX 1
#define PLAYER_WEBRADIO 2
#define PLAYER_BT 3
#define PLAYER_EXT 4

#define OUTPUT_SI47XX 2
#define OUTPUT_VS1053 1
#define OUTPUT_AUX 3
#define OUTPUT_AUX2 4

class Radio
{

    private:
        TwoWire *_i2cwire;
        Speaker *_spk;

        ChannelSwitch *_channel;
        FMTuner4735 *_fmtuner;
        
        //BlueToothPlayer *_bluetoothplayer;

        PreselectLeds *_preselectLeds;
        PreselectButtons *_preselectButtons;
        ChannelButtons *_channelButtons;
        Clock *_clock;
        FrequencyDisplay *_freq_display;
        
        ClockButtons *_clockButtons;
        
        uint8_t _currentPreset = 0;
        uint8_t _currentInput = 0;  // The input button pressed on the radio
        uint8_t _currentPlayer = 0; // The selected player or audio source
        uint8_t _currentOutput = 0; // The selected output channel

        bool mqttsetup = false;
        bool setupdone = false;
        
        void setupMQTT();
        unsigned long _lastClockUpdate;
        unsigned long _lastDisplayUpdate;
        unsigned long _lastMQTTUpdate;
        
        String _frequencyDisplayText;
        String _clockDisplayText0;
        String _clockDisplayText1;

        FanController pwmFan1;

        TaskHandle_t Task1;

        int _powersavemode = 0;
    public:
        void Setup();
        char Loop();
        void SecondaryLoop();
        void ExecuteCommand(char ch);
        void SwitchInput(uint8_t newinput);
        void ShowPercentage(int value, int max);
        void EnterPowerSaveMode(int lvl);
        void Stop();
        void Start();

        float LoopTime = 0;
        bool OTAOnly = false;

        InternetRadio *_inetRadio;
};

extern Radio _radio;

#endif