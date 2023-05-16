#include <Arduino.h>
#include "webserial.hpp"
#include <ESP32Encoder.h>

#ifndef ROTARYENCODER_H
#define ROTARYENCODER_H

class RotaryEncoderClass
{
    private:
        int64_t counter = 0;
        int64_t lastCounter = 0;

        ESP32Encoder encoder;
        uint8_t _sw;

        int SW_OLD = 0;
        
        unsigned long _lastread;
        bool running = false;

    public:
        void Setup(uint8_t cw, uint8_t ccw, uint8_t sw);
        void Loop();
        bool SwitchPressed = false;
        int64_t GetCounter();
};

extern RotaryEncoderClass RotaryEncoder;

#endif