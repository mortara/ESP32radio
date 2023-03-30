#include <Arduino.h>
#include <Wire.h>
#include <SI470X.h>
#include <Preferences.h>

#define RESET_PIN 13              // GPIO12

// I2C bus pin on ESP32
#define ESP32_I2C_SDA 21     // GPIO21
#define ESP32_I2C_SCL 22     // GPIO22 


class FMTuner
{
    private:
        SI470X  *radio;
        uint16_t channel = 9870;
        uint16_t volume = 14;
        char _seekmode = '0';
        long _seektimer = 0;
        char rdsBuffer[10];
        uint16_t current_station_preset = 0;
        uint16_t station_presets[8];
        bool _savemode = false;

    public:
        FMTuner();
        void Loop(char ch);
        void Stop();
        void Start();
        void DisplayInfo();
        void SaveCurrentChannel(int preset);
        void SwitchPreset(int num);
        void SetSaveMode(bool onoff);
        void SavePresets();
};