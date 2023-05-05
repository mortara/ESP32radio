#include "clock.hpp"

Clock::Clock(TwoWire *wire) : i2cdevice(wire, 0x68)
{
    _rtc = new RTC_DS3231();

    if(_rtc->begin(wire))
    {
        Serial.println("Clock found! ");
        char buf2[] = "DD.MM.YYYY hh:mm:ss";
        Serial.println(_rtc->now().toString(buf2));
        _active = true;
    }
    
    _lastUpdate = millis();
}

bool Clock::SetByNTP()
{
    Serial.println("Setting time by NTP Server");

    configTime(0,0, "ptbtime1.ptb.de");

    setenv("TZ","CET-1CEST,M3.5.0,M10.5.0/3",1);
    tzset();

    tm time;

    if(!getLocalTime(&time))
    {
        Serial.println("Could not get local time!");
        return false;
    }

    if(_active)
        return true;

    DateTime now(time.tm_year, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);

    _rtc->adjust(now); 
    _timeset = true;

    return true;
}

DateTime Clock::Now()
{
    if(!_active)
        return DateTime();

    return _rtc->now();
}

void Clock::Loop()
{
    unsigned long now = millis();
    if(now - _lastUpdate < 15000)
        return;

    _lastUpdate = now;

    if(!_timeset && WiFi.isConnected())
    {
        if(!SetByNTP())
            return;
    }

    /*char buf2[] = "DD.MM.YYYY hh:mm:ss";
    Serial.println(_rtc->now().toString(buf2));*/
}