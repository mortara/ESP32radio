#include <Arduino.h>
#include "RTClib.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <time.h>

class Clock
{
    private:
        RTC_DS3231 *_rtc;
        
        bool _timeset = false;
        unsigned long _lastUpdate; 

    public:
        Clock(TwoWire *wire);

        bool SetByNTP();
        DateTime Now();
        void Loop();
};

