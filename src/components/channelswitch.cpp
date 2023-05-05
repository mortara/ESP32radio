#include "channelswitch.hpp"

ChannelSwitch::ChannelSwitch(TwoWire *twowire, uint8_t adr) : i2cdevice(twowire, adr)
{
    if(!isActive())
    {
        Serial.println("Channel switch not found");
        _running = false;
        return;
    }

    Serial.println("Initializing Channel switcher");
    _address = adr;

    _i2cwire = twowire;
    _relais = new PCF8574(_i2cwire,_address);
    _relais->begin();
    _running = true;

    _relais->pinMode(P0, OUTPUT, offval);
    _relais->pinMode(P1, OUTPUT, offval);
    _relais->pinMode(P2, OUTPUT, offval);
    _relais->pinMode(P3, OUTPUT, offval);
    _relais->pinMode(P4, OUTPUT, offval);
    _relais->pinMode(P5, OUTPUT, offval);
    _relais->pinMode(P6, OUTPUT, offval);
    _relais->pinMode(P7, OUTPUT, offval);

}

void ChannelSwitch::TurnAllOff()
{
    Serial.println("Turn off all input channels");
    if(!_running)
        return;

    PCF8574::DigitalInput digitalInput;

    digitalInput.p0 = offval;
    digitalInput.p1 = offval;
    digitalInput.p2 = offval;
    digitalInput.p3 = offval;
    digitalInput.p4 = offval;
    digitalInput.p5 = offval;
    digitalInput.p6 = offval;
    digitalInput.p7 = offval;

    _relais->digitalWriteAll(digitalInput);
}

void ChannelSwitch::SetChannel(uint8_t channel)
{
    Serial.println("Switching to channel " + String(channel));

    if(!_running)
    {
        Serial.println("ERROR: relais module NOT started!");
        return;
    }

    PCF8574::DigitalInput digitalInput;

    digitalInput.p0 = offval;
    digitalInput.p1 = offval;
    digitalInput.p2 = offval;
    digitalInput.p3 = offval;
    digitalInput.p4 = offval;
    digitalInput.p5 = offval;
    digitalInput.p6 = offval;
    digitalInput.p7 = offval;

    int rs = (channel - 1) * 2;
    int rs2 = rs +1;

    switch(channel)
    {
        case 1:
            digitalInput.p0 = onval;
            digitalInput.p1 = onval;
            break;
        case 2:
            digitalInput.p2 = onval;
            digitalInput.p3 = onval;
            break;
        case 3:
            digitalInput.p4 = onval;
            digitalInput.p5 = onval;
            break;
        case 4:
            digitalInput.p6 = onval;
            digitalInput.p7 = onval;
            break;
    }

    _relais->digitalWriteAll(digitalInput);

    _channel = channel;

}