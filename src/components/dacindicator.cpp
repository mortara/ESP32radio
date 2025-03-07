#include "dacindicator.hpp"

void DACIndicator::Setup(uint8_t channel, uint16_t minvalue, uint16_t maxvalue, uint16_t startvalue)
{
    pmLogging.LogLn("Initializing DAC indicator " + String(channel));

    _max = maxvalue;
    _min = minvalue;
    _pin = channel;

    return;

    //pinMode(_pin, OUTPUT);
    //dacWrite(_pin,0);
    _current = -1;
}

void DACIndicator::SetValue(uint16_t value)
{
    if(_current == value)
        return;

    
    float r = (float)_max - (float)_min;
    float d = (float)value - (float)_min;

    float newvoltage = d / r * 255.0f;
    if(newvoltage < 0)
    {
        pmLogging.LogLn("DAC Voltage below zero! " + String(value) + ">" + String(newvoltage));
        newvoltage = 0;
    }
    _current_voltage = (uint8_t)newvoltage;

    //long pwmValue = map(value, 0, 1023, _min,_max);
    //Serial.println("DAC PIN: " + String(_pin) +  " " + String(value) + " = " + String(v)); 
    //dacWrite(_pin, _current_voltage);
    
    /*switch(_pin)
    {
        case 0:
            analogWrite(_pin, v);
            //dac_output_voltage(DAC_CHANNEL_1, v); 
            break;
        case 1:
            dac_output_voltage(DAC_CHANNEL_2, v); break;
    }*/

    _current = value;
}

void DACIndicator::SetRange(uint16_t min, uint16_t max)
{
    _max = max;
    _min = min;
    pmLogging.LogLn("Set PWM range: " + String(_min) + " -> " + String(_max));
}

void DACIndicator::DisplayInfo()
{
    pmLogging.LogLn("DAC: " + String(_pin) + " Min: " + String(_min) + " Max: " + String(_max) + " Cur: " + String(_current) + " => " + String(_current_voltage));
    
}

DACIndicator FrequencyIndicator;
DACIndicator SignalIndicator;