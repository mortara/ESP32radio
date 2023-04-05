#include "clock.hpp"


Clock::Clock(TwoWire *wire)
{
    _rtc = new RTC_DS3231();
    if(_rtc->begin(wire))
    {
        Serial.println("Clock found! ");
        char buf2[] = "YYMMDD-hh:mm:ss";
        Serial.println(_rtc->now().toString(buf2));
        
    }
    WiFiUDP ntpUDP;
    _timeClient = new NTPClient(ntpUDP, "us.pool.ntp.org", -7*3600);
}

bool Clock::SetByNTP()
{
    _timeClient->update();
    unsigned long epochTime = _timeClient->getEpochTime()-946684800UL;

    DateTime now(epochTime);
    _rtc->adjust(now); 
    return false;
}

DateTime Clock::Now()
{
    return _rtc->now();
}