#include "pwmindicator.hpp"

PWMIndicator::PWMIndicator(uint8_t pwmpin, long minvalue, long maxvalue, long startvalue)
{
    _max = maxvalue;
    _min = minvalue;
    _pin = pwmpin;

    pinMode(_pin, OUTPUT);
    dacWrite(_pin, 0);
}

void PWMIndicator::SetValue(long value)
{
    double r = _max - _min;
    uint8_t v = (uint8_t)((double)(value - _min) / r * 255.0);

    //long pwmValue = map(value, 0, 1023, _min,_max);
    Serial.print(String(value) + " = pwm->"); 
    Serial.println(String(v)); 
    //dacWrite(_pin,v);
    dac_output_voltage(DAC_CHANNEL_1, v); 
    delay(100);
    _current = value;
}

void PWMIndicator::SetRange(long min, long max)
{
    
    _max = max;
    _min = min;
    Serial.println("Set PWM range: " + String(_min) + " -> " + String(_max));
}