#include "preselect_buttons.hpp"


PreselectButtons::PreselectButtons(TwoWire* twowire, uint8_t adr) : i2cdevice(twowire, adr)
{
    if(!isActive())
    {
        WebSerialLogger.println("preselect buttons not found!");
        return;
    }

    WebSerialLogger.println("Initializing preselect buttons");

    _pcf8574 = new PCF8574(twowire,adr);
    _pcf8574->begin();

    _pcf8574->pinMode(P0, INPUT);
    _pcf8574->pinMode(P1, INPUT);
    _pcf8574->pinMode(P2, INPUT);
    _pcf8574->pinMode(P3, INPUT);
    _pcf8574->pinMode(P4, INPUT);
    _pcf8574->pinMode(P5, INPUT);
    _pcf8574->pinMode(P6, INPUT);
    _pcf8574->pinMode(P7, INPUT);

    _lastRead = millis();
}

int PreselectButtons::Loop()
{
    unsigned long now = millis();
    if(now - _lastRead < 200UL || !isActive())
        return 0;
    _lastRead = now;
        
    PCF8574::DigitalInput input = _pcf8574->digitalReadAll();

    int result = input.p0 + 2 * input.p1 + 4 * input.p2 + 8 * input.p3 + 16 * input.p4;
    result += 32 * input.p5 + 64 * input.p6 + 128 * input.p7;

    //Serial.println(String(input.p0)  +" " + String(input.p1) +" " + String(input.p2) +" " + String(input.p3) +" " + String(input.p4) +" " + String(input.p5) +" " + String(input.p6) +" " + String(input.p7));
    //delay(100);

    result = 255 - result;
    return result;
}
