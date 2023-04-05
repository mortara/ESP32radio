#include <Arduino.h>
#include "RTClib.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>

class Clock
{
    private:
        RTC_DS3231 *_rtc;
        NTPClient *_timeClient;
    public:
        Clock(TwoWire *wire);

        bool SetByNTP();
        DateTime Now();
};

