#include "clock_buttons.hpp"


ClockButtons::ClockButtons(TwoWire &twowire, uint8_t adr) : i2cdevice(twowire, adr)
{
    if(!isActive())
    {
        WebSerialLogger.println("clock buttons not found!");
        return;
    }

    WebSerialLogger.println("Initializing clock buttons");

    _pcf = new PCF8574(&twowire,adr);
    _pcf->pinMode(P0, INPUT_PULLUP);
    _pcf->pinMode(P7, INPUT_PULLUP);
    _pcf->begin();
    

    _lastRead = millis();
}

void ClockButtons::DisplayInfo()
{
    if(!isActive())
        return;

    PCF8574::DigitalInput input = _pcf->digitalReadAll();

    WebSerialLogger.println("Clockbuttons: " + String(input.p7));

    /*WebSerialLogger.println("Clockbuttons: " + String(_pcf8755->digitalRead(0)));
    WebSerialLogger.println("Clockbuttons: " + String(_pcf8755->digitalRead(1)));
    WebSerialLogger.println("Clockbuttons: " + String(_pcf8755->digitalRead(2)));
    WebSerialLogger.println("Clockbuttons: " + String(_pcf8755->digitalRead(3)));
    WebSerialLogger.println("Clockbuttons: " + String(_pcf8755->digitalRead(4)));
    WebSerialLogger.println("Clockbuttons: " + String(_pcf8755->digitalRead(5)));
    WebSerialLogger.println("Clockbuttons: " + String(_pcf8755->digitalRead(6)));
    WebSerialLogger.println("Clockbuttons: " + String(_pcf8755->digitalRead(7)));

    WebSerialLogger.println("Clockbuttons: " + String(read.p0) + String(read.p1) +String(read.p2) +String(read.p3) +String(read.p4) +String(read.p5) +String(read.p6) +String(read.p7));*/
}

int ClockButtons::readInputs()
{
    if(!isActive())
        return 0;


    int button = 0;
    
    PCF8574::DigitalInput read = _pcf->digitalReadAll();
    //WebSerialLogger.println(String(read.p0) + String(read.p1) +String(read.p2) +String(read.p3) +String(read.p4) +String(read.p5) +String(read.p6) +String(read.p7));
    return button;
}


int ClockButtons::Loop()
{
    unsigned long now = millis();
    if(now - _lastRead < 500)
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