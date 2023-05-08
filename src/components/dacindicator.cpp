#include "dacindicator.hpp"

DACIndicator::DACIndicator(uint8_t channel, uint16_t minvalue, uint16_t maxvalue, uint16_t startvalue)
{
    WebSerialLogger.println("Initializing DAC indicator " + String(channel));

    _max = maxvalue;
    _min = minvalue;
    _pin = channel;

    pinMode(_pin, OUTPUT);
    dacWrite(_pin,0);
    
}

void DACIndicator::SetValue(uint16_t value)
{
    uint16_t r = _max - _min;
    _current_voltage = (uint8_t)((double)(value - _min) / (double)r * 255.0);

    //long pwmValue = map(value, 0, 1023, _min,_max);
    //Serial.println("DAC PIN: " + String(_pin) +  " " + String(value) + " = " + String(v)); 
    dacWrite(_pin, _current_voltage);
    
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
    WebSerialLogger.println("Set PWM range: " + String(_min) + " -> " + String(_max));
}

void DACIndicator::Loop(char ch)
{
    if(ch == 'y')
    {
        WebSerialLogger.println("DAC: " + String(_pin) + " Min: " + String(_min) + " Max: " + String(_max) + " Cur: " + String(_current) + " => " + String(_current_voltage));
    }
}