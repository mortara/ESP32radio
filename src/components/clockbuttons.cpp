#include "clock_buttons.hpp"


ClockButtons::ClockButtons(TwoWire* twowire, uint8_t adr) : i2cdevice(twowire, adr)
{
    if(!isActive())
    {
        WebSerialLogger.println("clock buttons not found!");
        return;
    }

    WebSerialLogger.println("Initializing clock buttons");

    _pcf = new PCF8574(twowire,adr);
    _pcf->pinMode(P0, INPUT);
    _pcf->pinMode(P1, INPUT);
    _pcf->pinMode(P2, INPUT);
    _pcf->pinMode(P3, INPUT);
    _pcf->pinMode(P4, INPUT);
    _pcf->pinMode(P5, INPUT);
    _pcf->pinMode(P6, INPUT);
    _pcf->pinMode(P7, INPUT);
    _pcf->begin();
    
    _lastRead = millis();
}

void ClockButtons::DisplayInfo()
{
    if(!isActive())
        return;

    PCF8574::DigitalInput input = _pcf->digitalReadAll();

    WebSerialLogger.println(String(input.p0)  +" " + String(input.p1) +" " + String(input.p2) +" " + String(input.p3) +" " + String(input.p4) +" " + String(input.p5) +" " + String(input.p6) +" " + String(input.p7));
}

int ClockButtons::readInputs()
{
    if(!isActive())
        return 0;


    int button = 0;
    button1_pressed = false;
    button2_pressed = false;
    PCF8574::DigitalInput read = _pcf->digitalReadAll();

    if(read.p0 == 0)
    {
        button1_pressed = true;
    }
    
    if(read.p1 == 0)
    {
        button2_pressed = true;
    }
    
    if(read.p2 == 0)
        button = 3;
    else if(read.p3 == 0)
        button = 4;
    else if(read.p4 == 0)
        button = 5;
    else if(read.p5 == 0)
        button = 6;
    else if(read.p6 == 0)
        button = 7;
    else if(read.p7 == 0)
        button = 8;
   
    return button;
}


int ClockButtons::Loop()
{
    unsigned long now = millis();
    if(now - _lastRead < 500UL)
        return 0;
    _lastRead = now;

    int button = readInputs();

    // Input is read twice for debouncing the switch!
    if(button != 0)
    {
        WebSerialLogger.println("Clockbutton " + String(button) + " pressed!");
        return button;
    }

    return 0;
}