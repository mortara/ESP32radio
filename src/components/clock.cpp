#include "clock.hpp"


Clock::Clock(TwoWire *wire)
{
    _rtc = new RTC_DS3231();
    if(_rtc->begin(wire))
    {
        Serial.println("Clock found! ");
        char buf2[] = "DD.MM.YYYY hh:mm:ss";
        Serial.println(_rtc->now().toString(buf2));
        
    }
    
    _lastUpdate = millis();
}

bool Clock::SetByNTP()
{
    Serial.println("Setting time by NTP Server");

    WiFiUDP ntpUDP;
    NTPClient _timeClient(ntpUDP, "ptbtime1.ptb.de");

    if(_timeClient.update())
        Serial.println(" ... success");
    else
    {
        Serial.println(" ... failed");
        return false;
    }

    unsigned long epochTime = _timeClient.getEpochTime()-946684800UL;

    DateTime now(epochTime);
    
    _rtc->adjust(now); 
    _timeset = true;

    _timeClient.end();
    return true;
}

DateTime Clock::Now()
{
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

    char buf2[] = "DD.MM.YYYY hh:mm:ss";
    Serial.println(_rtc->now().toString(buf2));
}