#include <Arduino.h>
#include "RTClib.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <time.h>
#include "i2cdevice.hpp"

class Clock : i2cdevice
{
    private:
        RTC_DS3231 *_rtc;
        
        bool _timeset = false;
        unsigned long _lastUpdate; 
        bool _active = false;
        
    public:
        Clock(TwoWire *wire);

        bool SetByNTP();
        DateTime Now();
        void Loop();
};

