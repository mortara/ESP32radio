#include "tunerbuttons.hpp"


void TunerButtonsClass::Setup(TwoWire *twowire, uint8_t adr)
{
    i2cdevice::Setup(twowire, adr);

    if(!isActive())
    {
        pmLogging.LogLn("Tuner buttons not found!");
        return;
    }

    pmLogging.LogLn("Initializing tuner buttons switcher");

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

int TunerButtonsClass::Loop()
{
    unsigned long now = millis();
    if(now - _lastRead < 200UL || !isActive())
        return 0;
    _lastRead = now;    
    
    PCF8574::DigitalInput input = _pcf8754->digitalReadAll();

    if(input.p0 == 0)
        SavePresetButtonPressed = true;
    else
        SavePresetButtonPressed = false;

    int result = 2 * input.p1 + 4 * input.p2 + 8 * input.p3 + 16 * input.p4;
    result += 32 * input.p5 + 64 * input.p6 + 128 * input.p7;

    result = 254 - result;

    /*Serial.println(String(input.p0)  +" " + String(input.p1) +" " + String(input.p2) +" " + String(input.p3));
    delay(300);*/

    return result;
}

TunerButtonsClass TunerButtons;