#include "speaker.hpp"


Speaker::Speaker(int pin)
{
    _relaispin = pin;

    Serial.println("\n\nSetup speaker");
    digitalWrite(_relaispin, HIGH);
    pinMode(_relaispin, OUTPUT);

    TurnOff();
}

void Speaker::TurnOn()
{
    Serial.println("Speaker ON");
    //GPIO.out 
    digitalWrite(_relaispin, LOW);
    _onoff = true;
}

void Speaker::TurnOff()
{
    Serial.println("Speaker OFF");
    digitalWrite(_relaispin, HIGH);
    _onoff = false;
}

void Speaker::ExecuteCommand(char ch)
{
    if (ch == 's') 
    {
        if(_onoff)
            TurnOff();
        else
            TurnOn();
    }
}