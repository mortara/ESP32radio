#include <Arduino.h>
#include <driver/dac.h>
#include "webserial.hpp"

#ifndef DACINDICATOR_H
#define DACINDICATOR_H


class DACIndicator
{
    private:
        uint16_t _max;
        uint16_t _min;
        uint16_t _current;
        uint8_t _current_voltage;
        uint8_t _pin;
        
    public:
        DACIndicator(uint8_t pwmpin, uint16_t minvalue, uint16_t maxvalue, uint16_t startvalue);

        void SetValue(uint16_t val);
        void SetRange(uint16_t min, uint16_t max);
        void Loop(char ch);
};

#endif