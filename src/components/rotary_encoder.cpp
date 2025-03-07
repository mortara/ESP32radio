#include "rotary_encoder.hpp"

void RotaryEncoderClass::Setup(uint8_t cw, uint8_t ccw, uint8_t sw)
{
    _sw = sw;

    encoder.attachSingleEdge(cw, ccw);
    encoder.setCount(0);
    pinMode(_sw, INPUT);
    _lastread = millis();
    pmLogging.LogLn("Rotary encoder started");
    running = true;
}

int64_t RotaryEncoderClass::GetCounter()
{
    if(!running)
      return 0;

    return encoder.getCount();
}

void RotaryEncoderClass::Loop()
{
    //counter = encoder.getCount();

    unsigned long now = millis();

    if(now - _lastread > 200UL)
    {
      
      _lastread = now;

      int SW = digitalRead(_sw);
      if(SW == 0 && SW_OLD != 0)
        SwitchPressed = true;
      else
        SwitchPressed = false;

      SW_OLD = SW;
    }
}

RotaryEncoderClass RotaryEncoder;