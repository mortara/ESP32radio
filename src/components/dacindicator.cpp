#include "dacindicator.hpp"

DACIndicator::DACIndicator(uint8_t channel, long minvalue, long maxvalue, long startvalue)
{
    Serial.println("Initializing DAC indicator " + String(channel));

    _max = maxvalue;
    _min = minvalue;
    _pin = channel;

    pinMode(_pin, OUTPUT);
    SetValue(0);
}

void DACIndicator::SetValue(long value)
{
    double r = _max - _min;
    uint8_t v = (uint8_t)((double)(value - _min) / r * 255.0);

    //long pwmValue = map(value, 0, 1023, _min,_max);
    Serial.println("DAC PIN: " + String(_pin) +  " " + String(value) + " = " + String(v)); 
    dacWrite(_pin,v);

    /*switch(_pin)
    {
        case 0:
            analogWrite(_pin, v);
            //dac_output_voltage(DAC_CHANNEL_1, v); 
            break;
        case 1:
            dac_output_voltage(DAC_CHANNEL_2, v); break;
    }*/
    
    delay(100);
    _current = value;
}

void DACIndicator::SetRange(long min, long max)
{
    
    _max = max;
    _min = min;
    Serial.println("Set PWM range: " + String(_min) + " -> " + String(_max));
}