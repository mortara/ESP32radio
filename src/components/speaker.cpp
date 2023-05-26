#include "speaker.hpp"


Speaker::Speaker(int pin)
{
    _relaispin = pin;

    WebSerialLogger.println("Setup speaker");
    digitalWrite(_relaispin, HIGH);
    pinMode(_relaispin, OUTPUT);
    _onoff = false;
}

void Speaker::TurnOn()
{
    WebSerialLogger.println("Speaker ON");
    //GPIO.out 
    digitalWrite(_relaispin, LOW);
    _onoff = true;
}

void Speaker::TurnOff()
{
    WebSerialLogger.println("Speaker OFF");
    digitalWrite(_relaispin, HIGH);
    _onoff = false;
}

void Speaker::Toggle()
{
    if(_onoff)
        TurnOff();
    else
        TurnOn();
}
