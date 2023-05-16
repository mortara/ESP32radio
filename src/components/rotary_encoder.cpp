#include "rotary_encoder.hpp"

void RotaryEncoderClass::Setup(uint8_t cw, uint8_t ccw, uint8_t sw)
{
    _sw = sw;

    encoder.attachHalfQuad(cw, ccw);
    encoder.setCount(0);
    pinMode(_sw, INPUT);
    _lastread = millis();
    WebSerialLogger.println("Rotary encoder started");
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

    if(now - _lastread > 200)
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