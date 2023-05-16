#include "clock.hpp"

Clock::Clock(TwoWire &wire) : i2cdevice(wire, 0x68)
{
    if(_rtc.begin(&wire))
    {
        WebSerialLogger.println("Clock found! ");
        _active = true;
    }
}

bool Clock::SetByNTP()
{
    WebSerialLogger.println("Setting time by NTP Server");

    configTime(0,0, "ptbtime1.ptb.de");

    setenv("TZ","CET-1CEST,M3.5.0,M10.5.0/3",1);
    tzset();

    tm time;

    if(!getLocalTime(&time))
    {
        WebSerialLogger.println("Could not get local time!");
        return false;
    }

    if(time.tm_year == 2000)
    {
        WebSerialLogger.println("Invalid DateTime received");
        return false;
    }

    if(!_active)
        return true;

    DateTime now(time.tm_year, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);

    _rtc.adjust(now); 
    _timeset = true;

    return true;
}

DateTime Clock::Now()
{
    if(!_active)
    {
        tm time;
        if(getLocalTime(&time))
        {
            return DateTime(time.tm_year, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
        }

        return DateTime();
    }

    return _rtc.now();
}

void Clock::DisplayInfo()
{
    WebSerialLogger.println("Clock active = " + String(_active));
    WebSerialLogger.println("Clock set by NTP = " + String(_timeset));
    WebSerialLogger.println("Current date = " + GetDateTimeString());
    WebSerialLogger.println("DS3231 time = " + String(_rtc.now().toString(_defaultformat)));
}

String Clock::GetDateTimeString(bool seconds)
{
    if(seconds)
        return Now().toString(_defaultformat);

    return Now().toString(_defaultshortformat);
}

bool Clock::IsSet()
{
    return _timeset;
}
