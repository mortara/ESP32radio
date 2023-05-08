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

    //Serial.println(String(input.p0)  +" " + String(input.p1) +" " + String(input.p2) +" " + String(input.p3) +" " + String(input.p4) +" " + String(input.p5) +" " + String(input.p6)+" " + String(input.p7));
    //delay(300);

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
        button = readInputs();

        WebSerialLogger.println("Button " + String(button) + " pressed!");
    }

    return button;
}