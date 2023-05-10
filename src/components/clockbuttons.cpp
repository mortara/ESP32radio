#include "clock_buttons.hpp"


ClockButtons::ClockButtons(TwoWire *twowire, uint8_t adr) : i2cdevice(twowire, adr)
{
    if(!isActive())
    {
        WebSerialLogger.println("channel buttons not found!");
        return;
    }

    WebSerialLogger.println("Initializing channel buttons");
    _address = adr;

    _i2cwire = twowire;
    _pcf8755 = new PCF8575(_i2cwire,_address);
    _pcf8755->begin();
   
    _pcf8755->pinMode(P0, INPUT);
    _pcf8755->pinMode(P1, INPUT);
    _pcf8755->pinMode(P2, INPUT);
    _pcf8755->pinMode(P3, INPUT);
    _pcf8755->pinMode(P4, INPUT);
    _pcf8755->pinMode(P5, INPUT);
    _pcf8755->pinMode(P6, INPUT);
    _pcf8755->pinMode(P7, INPUT);
    _pcf8755->pinMode(P8, INPUT);
    _pcf8755->pinMode(P9, INPUT);
    _pcf8755->pinMode(P10, INPUT);
    _pcf8755->pinMode(P11, INPUT);
    _pcf8755->pinMode(P12, INPUT);
    _pcf8755->pinMode(P13, INPUT);
    _pcf8755->pinMode(P14, INPUT);
    _pcf8755->pinMode(P15, INPUT);

    _lastRead = millis();
}

int ClockButtons::readInputs()
{
    if(!isActive())
        return 0;

    PCF8575::DigitalInput input = _pcf8755->digitalReadAll();

    int button = 0;

    if(input.p0 == 1)
        button = 1;
    else if(input.p1 == 1)
        button = 2;
    else if(input.p2 == 1)
        button = 3;
    else if(input.p3 == 1)
        button = 4;
    else if(input.p4 == 1)
        button = 5;
    else if(input.p5 == 1)
        button = 6;
    else if(input.p6 == 1)
        button = 7;
    else if(input.p7 == 1)
        button = 8;
    else if(input.p8 == 1)
        button = 9;
    else if(input.p9 == 1)
        button = 10;
    else if(input.p10 == 1)
        button = 11;
    else if(input.p11 == 1)
        button = 12;
    else if(input.p12 == 1)
        button = 13;
    else if(input.p13 == 1)
        button = 14;
    else if(input.p14 == 1)
        button = 15;
    else if(input.p15 == 1)
        button = 16;

    return button;
}


int ClockButtons::Loop()
{
    unsigned long now = millis();
    if(now - _lastRead < 100)
        return 0;
    _lastRead = now;

    int button = readInputs();

    // Input is read twice for debouncing the switch!
    if(button != 0)
    {
        if(readInputs() == button)
        {
            WebSerialLogger.println("Button " + String(button) + " pressed!");
            return button;
        }
    }

    return 0;
}