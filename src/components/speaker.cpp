#include "speaker.hpp"


Speaker::Speaker(int pin)
{
    _relaispin = pin;

    pmLogging.LogLn("Setup speaker");

    pinMode(_relaispin, OUTPUT);
    digitalWrite(_relaispin, HIGH);
    pinMode(_relaispin, OUTPUT);
    _onoff = false;
}

void Speaker::TurnOn()
{
    pmLogging.LogLn("Speaker ON");
    //GPIO.out 
    digitalWrite(_relaispin, LOW);
    _onoff = true;
}

void Speaker::TurnOff()
{
    pmLogging.LogLn("Speaker OFF");
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
