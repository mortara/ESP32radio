#include "preselect_leds.hpp"


PreselectLeds::PreselectLeds(TwoWire &twowire, uint8_t adr) : i2cdevice(twowire, adr)
{
    if(!isActive())
    {
        WebSerialLogger.println("preselect led strip not found!");
        return;
    }

    WebSerialLogger.println("Initializing preselect led strip");
    
    _pcf8754 = new PCF8574(&twowire,adr);
    _pcf8754->begin();
   
    _pcf8754->pinMode(P0, OUTPUT, LOW);
    _pcf8754->pinMode(P1, OUTPUT, LOW);
    _pcf8754->pinMode(P2, OUTPUT, LOW);
    _pcf8754->pinMode(P3, OUTPUT, LOW);
    _pcf8754->pinMode(P4, OUTPUT, LOW);
    _pcf8754->pinMode(P5, OUTPUT, LOW);
    _pcf8754->pinMode(P6, OUTPUT, LOW);
    _pcf8754->pinMode(P7, OUTPUT, LOW);

    SetLed(99);

}

void PreselectLeds::SetLed(uint8_t num)
{
    if(!isActive())
        return;

    WebSerialLogger.println("preselect led on: " + String(num));

    PCF8574::DigitalInput digitalInput;

    digitalInput.p0 = LOW;
    digitalInput.p1 = LOW;
    digitalInput.p2 = LOW;
    digitalInput.p3 = LOW;
    digitalInput.p4 = LOW;
    digitalInput.p5 = LOW;
    digitalInput.p6 = LOW;
    digitalInput.p7 = LOW;

    _pcf8754->digitalWriteAll(digitalInput);

    if(num < 0 || num > 8)
        return;

    delay(10);
    switch(num)
    {
        case 0:
            _pcf8754->digitalWrite(P0, HIGH); break;
        case 1:
            _pcf8754->digitalWrite(P1, HIGH); break;
        case 2:
            _pcf8754->digitalWrite(P2, HIGH); break;
        case 3:
            _pcf8754->digitalWrite(P3, HIGH); break;
        case 4:
            _pcf8754->digitalWrite(P4, HIGH); break;
        case 5:
            _pcf8754->digitalWrite(P5, HIGH); break;
        case 6:
            _pcf8754->digitalWrite(P6, HIGH); break;
        case 7:
            _pcf8754->digitalWrite(P7, HIGH); break;
    }
    
}
