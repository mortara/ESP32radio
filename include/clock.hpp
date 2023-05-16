#include <Arduino.h>
#include "RTClib.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <time.h>
#include "i2cdevice.hpp"

#ifndef CLOCK_H
#define CLOCK_H

class Clock : i2cdevice
{
    private:
        RTC_DS3231 _rtc;
        
        bool _timeset = false;
        bool _active = false;
        
        char _defaultformat[20] = "DD.MM.YYYY hh:mm:ss";
        char _defaultshortformat[17] = "DD.MM.YYYY hh:mm";

    public:
        Clock(TwoWire &wire);

        bool SetByNTP();
        DateTime Now();
        String GetDateTimeString(bool seconds = true);
        bool IsSet();
        void DisplayInfo();

};

#endif
