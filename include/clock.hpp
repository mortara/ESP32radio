#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <time.h>
#include "i2cdevice.hpp"
#include <DS3232RTC.h>

#ifndef CLOCK_H
#define CLOCK_H

class Clock : i2cdevice
{
    private:
        DS3232RTC _rtc;
        bool _timeset = false;
        bool _active = false;
    
    public:
        Clock(TwoWire &wire);

        bool SetByNTP();
        String TimeTToString(tmElements_t time, bool seconds);
        String TimeTToString(tm time, bool seconds);
        String GetDateTimeString(time_t time, bool seconds);
        String GetDateTimeString(bool seconds = true);

        String FormatDateTime(int year, int month, int day, int hour, int minute, int seconds, bool showseconds);

        void DisplayInfo();

};

#endif
