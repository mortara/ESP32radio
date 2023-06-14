#include <Arduino.h>
//#include <driver/dac.h>
#include "webserial.hpp"

#ifndef DACINDICATOR_H
#define DACINDICATOR_H


class DACIndicator
{
    private:
        uint16_t _max = 255;
        uint16_t _min = 0;
        uint16_t _current = 0;
        uint8_t _current_voltage = 0;
        uint8_t _pin = 0;
        
    public:
        void Setup(uint8_t pwmpin, uint16_t minvalue, uint16_t maxvalue, uint16_t startvalue);
        void SetValue(uint16_t val);
        void SetRange(uint16_t min, uint16_t max);
        void DisplayInfo();
};

extern DACIndicator FrequencyIndicator;
extern DACIndicator SignalIndicator;


#endif