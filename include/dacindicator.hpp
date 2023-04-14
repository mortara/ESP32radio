#include <Arduino.h>
#include <driver/dac.h>

#ifndef DACINDICATOR_H
#define DACINDICATOR_H


class DACIndicator
{
    private:
        long _max;
        long _min;
        long _current;
        uint8_t _pin;

    public:
        DACIndicator(uint8_t pwmpin, long minvalue, long maxvalue, long startvalue);

        void SetValue(long val);
        void SetRange(long min, long max);
};

#endif