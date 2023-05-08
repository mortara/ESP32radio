#include <Arduino.h>
#include "webserial.hpp"

class RotaryEncoder
{
    private:
        volatile int counter = 0;
        volatile int lastCounter = 0;

        volatile uint8_t _prevValueAB = 0;    //previouse state of "A"+"B"
        volatile uint8_t _currValueAB = 0;    //current   state of "A"+"B"

        uint8_t ENC_A;
        uint8_t ENC_B;
        uint8_t _sw;
        int SW_OLD = 0;
        void read_encoder();

        

    public:
        RotaryEncoder(uint8_t cw, uint8_t ccw, uint8_t sw);
        void Loop();

        bool SwitchPressed = false;
        int GetCounter();
};