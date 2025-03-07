#include <Arduino.h>
#include "pmCommonLib.hpp"

class FanController
{
        uint8_t pwmpin = 0;
        uint8_t pwmchannel = 0;
    public:
        void Begin(uint8_t pin);
        void StartFan();
        void StopFan();
        bool FanState = false;

};