#include <Arduino.h>
#include <Wire.h>
#include <SI4735.h>
#include <Preferences.h>
#include "dacindicator.hpp"
#include "mqtt.hpp"
#include "webserial.hpp"

#ifndef FMTUNER4735_H
#define FMTUNER4735_H

#define SI7435_RESET_PIN 2              // GPIO12

// I2C bus pin on ESP32
#define ESP32_I2C_SDA 21     // GPIO21
#define ESP32_I2C_SCL 22     // GPIO22 

#define AM_FUNCTION 1
#define FM_FUNCTION 0


#define FM_BAND_TYPE 0
#define MW_BAND_TYPE 1
#define SW_BAND_TYPE 2
#define LW_BAND_TYPE 3

#define FM 0
#define LSB 1
#define USB 2
#define AM 3
#define LW 4



class FMTuner4735
{
    private:
        SI4735  *_radio;
        bool _active = false;

        DACIndicator *_pwmindicator_freq;
        DACIndicator *_pwmindicator_signal;

        bool mqttsetup = false;
        void setupMQTT();
        void sendMQTTState();
        uint8_t _band = 6;
        uint16_t currentFrequency = 0;
        uint16_t previousFrequency = 0;
        uint8_t currentMode = FM;

        uint16_t _volume = 63;
        char _seekmode = '0';
        unsigned long _seektimer = 0;
        char _rdsBuffer[10];
        uint8_t _current_station_preset = 0;
        uint16_t _station_presets[8];
        bool _savemode = false;
       
        uint8_t _currentBand = 99;
        uint8_t _smallstep = 5;
        uint8_t _step = 10;
        unsigned long _lastUpdate;
        
        uint16_t setFrequency(u_int16_t freq);

        char *rdsMsg = NULL;
        char *stationName = NULL;
        char *rdsTime = NULL;

        String RDSMessage;
        String RDSStationName;
        String RDSTime;
        unsigned long _lastRDSUpdate;
        
        uint8_t clockdisplaypage = 0;
        unsigned long clockdisplaypagetimer = 0;
        unsigned long frequencychangetimeout = 0;

        unsigned long _lastRotaryRead = 0;
        int64_t _lastRotaryCount = 0;

        void checkRDS();

        typedef struct
        {
            uint8_t idx;      // SI473X device bandwidth index
            const char *desc; // bandwidth description
        } Bandwidth;

        Bandwidth bandwidthAM[7] = {
            {4, "1.0"},
            {5, "1.8"},
            {3, "2.0"},
            {6, "2.5"},
            {2, "3.0"},
            {1, "4.0"},
            {0, "6.0"}
        };

        Bandwidth bandwidthFM[5] = {
            {0, "AUT"}, // Automatic - default
            {1, "110"}, // Force wide (110 kHz) channel filter.
            {2, " 84"},
            {3, " 60"},
            {4, " 40"}};



        int tabAmStep[6] = {1,    // 0
                        5,    // 1
                        9,    // 2
                        10,   // 3
                        50,   // 4
                        100}; // 5

        const int lastAmStep = (sizeof tabAmStep / sizeof(int)) - 1;
        int idxAmStep = 3;

        int tabFmStep[3] = {5, 10, 20};
        const int lastFmStep = (sizeof tabFmStep / sizeof(int)) - 1;
        
        uint16_t currentStepIdx = 1;

        const char *bandwidth[7] = {"6", "4", "3", "2", "1", "1.8", "2.5"};
        typedef struct
        {
            const char *bandName;   // Band description
            uint8_t bandType;       // Band type (FM, MW or SW)
            uint16_t minimumFreq;   // Minimum frequency of the band
            uint16_t maximumFreq;   // maximum frequency of the band
            uint16_t currentFreq;   // Default frequency or current frequency
            int8_t currentStepIdx;  // Idex of tabStepAM:  Defeult frequency step (See tabStepAM)
            int8_t bandwidthIdx;    // Index of the table bandwidthFM, bandwidthAM or bandwidthSSB;
            uint8_t disableAgc;
            int8_t agcIdx;
            int8_t agcNdx;
            int8_t avcIdx; 
            int8_t softMuteMaxAttenuation;
        } Band;

        Band _bands[7] = {
            {"LW", LW_BAND_TYPE, 150, 279, 150, 3, 4, 0, 0, 0, 32, 0},
            {"MW", MW_BAND_TYPE, 300, 3500, 783, 2, 4, 0, 0, 0, 32, 0},
            {"SW1", SW_BAND_TYPE, 4000, 5500, 4885, 1, 4, 1, 0, 0, 32, 0},
            {"SW2", SW_BAND_TYPE, 5500, 6500, 6000, 1, 4, 1, 0, 0, 32, 0},
            {"SW3", SW_BAND_TYPE, 7200, 8000, 7200, 1, 4, 1, 0, 0, 40, 0},
            {"UKW", FM_BAND_TYPE, 8790, 10800, 10390, 1, 0, 1, 0, 0, 0, 0},
            {"ALL", SW_BAND_TYPE, 150, 30000, 15000, 0, 4, 1, 0, 0, 48, 0} // All band. LW, MW and SW (from 150kHz to 30MHz)
        };

    public:
        FMTuner4735(DACIndicator *freq, DACIndicator *signal);
        void Loop(char ch);
        void Stop();
        void Start(uint8_t band);
        void SwitchPreset(uint8_t num);
        void SwitchBand(uint8_t band);
        void DisplayInfo();
        void SaveCurrentChannel(uint8_t preset);
        void SetSaveMode(bool onoff);
        void SavePresets();
        void LoadPresets();
        String GetFreqDisplayText();
        String GetClockDisplayText();
        uint16_t GetBandMin();
        uint16_t GetBandMax();
};

#endif