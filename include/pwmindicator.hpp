#include <Arduino.h>
#include <driver/dac.h>

#ifndef PWMINDICATOR_H
#define PWMINDICATOR_H


class PWMIndicator
{
    private:
        long _max;
        long _min;
        long _current;
        uint8_t _pin;

    public:
        PWMIndicator(uint8_t pwmpin, long minvalue, long maxvalue, long startvalue);

        void SetValue(long val);
        void SetRange(long min, long max);
};

#endif