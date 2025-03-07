#include "channelbuttons.hpp"

ChannelButtons::ChannelButtons(TwoWire* twowire, uint8_t adr) : i2cdevice(twowire, adr)
{
    if(!isActive())
    {
        pmLogging.LogLn("channel buttons not found!");
        return;
    }

    pmLogging.LogLn("Initializing channel buttons");
   
    _pcf8754 = new PCF8574(twowire,adr);
    _pcf8754->begin();
   
    _pcf8754->pinMode(P0, INPUT);
    _pcf8754->pinMode(P1, INPUT);
    _pcf8754->pinMode(P2, INPUT);
    _pcf8754->pinMode(P3, INPUT);
    _pcf8754->pinMode(P4, INPUT);
    _pcf8754->pinMode(P5, INPUT);
    _pcf8754->pinMode(P6, INPUT);
    _pcf8754->pinMode(P7, INPUT);

    _lastRead = millis();
}

int ChannelButtons::Loop()
{
    unsigned long now = millis();
    if(now - _lastRead < 400 || !isActive())
        return 0;
    _lastRead = now;

    PCF8574::DigitalInput input = _pcf8754->digitalReadAll();

    int channel = 0;

    if(input.p0 == 0)
        channel = 1;
    else if(input.p1 == 0)
        channel = 2;
    else if(input.p2 == 0)
        channel = 3;
    else if(input.p3 == 0)
        channel = 4;
    else if(input.p4 == 0)
        channel = 5;
    else if(input.p5 == 0)
        channel = 6;
    else if(input.p6 == 0)
        channel = 7;
    else if(input.p7 == 0)
        channel = 8;

    if(channel != currentchannel)
    {   
        pmLogging.LogLn("Channel button switched from " + String(currentchannel) + " to " + String(channel));
        currentchannel = channel;
        return channel;
    }

    return 0;
}