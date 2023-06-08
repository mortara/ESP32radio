#include "clock.hpp"


Clock::Clock(TwoWire &wire) : i2cdevice(wire, 0x68)
{
    _rtc.begin();

    setSyncProvider(_rtc.get);   // the function to get the time from the RTC

    setenv("TZ","CET-1CEST,M3.5.0,M10.5.0/3",1);
    tzset();

    _ce = new Timezone(CEST, CET);

    if(timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");
    _active = true;
    
}

bool Clock::SetByNTP()
{
    WebSerialLogger.println("Setting time by NTP Server");

    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "ptbtime1.ptb.de", "ptbtime2.ptb.de", "ptbtime3.ptb.de");
    //configTime(0,0, "ptbtime1.ptb.de");

    tm time = {0};
    time.tm_isdst = -1;

    if(!getLocalTime(&time))
    {
        WebSerialLogger.println("Could not get local time!");
        return false;
    }

    if(!_active)
        return true;

    WebSerialLogger.println("dst = " + String(time.tm_isdst));

    time_t now = mktime(&time);
    
    _rtc.set(now); 
    
    _timeset = true;
    time_t timeSinceEpoch = mktime(&time);
    now = _rtc.get();
    tmElements_t tme;
    _rtc.read(tme);
    WebSerialLogger.println("tme = " + TimeTToString(tme, true));
    WebSerialLogger.println("now = " + GetDateTimeString(now));
    WebSerialLogger.println("timeSinceEpoch = " + GetDateTimeString(timeSinceEpoch));

    return true;
}

void Clock::DisplayInfo()
{
    if(_active)
        WebSerialLogger.println("Clock active = YES");
    else
        WebSerialLogger.println("Clock active = NO");

    if(_timeset)
        WebSerialLogger.println("Clock set by NTP = YES" );
    else
        WebSerialLogger.println("Clock set by NTP = NO" );

    WebSerialLogger.println("Current date = " + GetDateTimeString());
    //WebSerialLogger.println("DS3231 time = " + String(_rtc.get()));
}

String Clock::TimeTToString(tmElements_t time, bool seconds)
{
    return FormatDateTime(time.Year, time.Month, time.Day, time.Hour, time.Minute,time.Second, seconds);
}

String Clock::TimeTToString(tm time, bool seconds)
{
    time_t nt = mktime(&time);
    return GetDateTimeString(nt,seconds);
    //return FormatDateTime(time.tm_year+1900, time.tm_mon+1, time.tm_mday, time.tm_hour, time.tm_min,time.tm_sec, seconds);
}

String Clock::GetDateTimeString(time_t time, bool seconds)
{
    return FormatDateTime(year(time), month(time), day(time), hour(time), minute(time),second(time), seconds);
}

String Clock::FormatDateTime(int year, int month, int day, int hour, int minute, int seconds, bool showseconds)
{
    String y = String(year);
    String m = String(month);
    if(m.length() == 1)
        m = "0" + m;

    String d = String(day);
    if(d.length() == 1)
        d = "0" + d;

    String h = String(hour);
    if(h.length() == 1)
       h  = "0" + h;

    String mi = String(minute);
    if(mi.length() == 1)
        mi = "0" + mi;

    String s = String(seconds);
    if(s.length() == 1)
        s = "0" + s;

    String result = d + "." + m + "." + y + " " + h + ":" + mi;
    if(showseconds)
        result = result + ":" + s;

    return result;
}

String Clock::GetDateTimeString(bool seconds)
{
    time_t nt2 = _ce->toLocal(_rtc.get());
    return GetDateTimeString(nt2, seconds);
}
