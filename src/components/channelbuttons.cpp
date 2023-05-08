#include "channelbuttons.hpp"

ChannelButtons::ChannelButtons(TwoWire *twowire, uint8_t adr) : i2cdevice(twowire, adr)
{
    if(!isActive())
    {
        WebSerialLogger.println("channel buttons not found!");
        return;
    }

    WebSerialLogger.println("Initializing channel buttons");
    _address = adr;

    _i2cwire = twowire;
    _pcf8754 = new PCF8574(_i2cwire,_address);
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

int ChannelButtons::readInputs()
{
    if(!isActive())
        return 0;

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

    //Serial.println(String(input.p0)  +" " + String(input.p1) +" " + String(input.p2) +" " + String(input.p3) +" " + String(input.p4) +" " + String(input.p5) +" " + String(input.p6)+" " + String(input.p7));
    //delay(300);

    return channel;
}


int ChannelButtons::Loop()
{
    unsigned long now = millis();
    if(now - _lastRead < 100)
        return 0;
    _lastRead = now;

    int result = 0;
    int channel = readInputs();

    // Input is read twice for debouncing the switch!
    if(channel != currentchannel)
    {   
        delay(50);
        channel = readInputs();
        if(channel != currentchannel)
        {
            WebSerialLogger.println("Channel button switched from " + String(currentchannel) + " to " + String(channel));
            currentchannel = channel;
            result = channel;
        }
    }

    return result;
}