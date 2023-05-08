#include "rotary_encoder.hpp"


RotaryEncoder::RotaryEncoder(uint8_t cw, uint8_t ccw, uint8_t sw)
{
    ENC_A = cw;
    ENC_B = ccw;
    _sw = sw;

     // Set encoder pins
    pinMode(ENC_A, INPUT);
    pinMode(ENC_B, INPUT);
    pinMode(_sw, INPUT);

    // Start the serial monitor to show output
    
    WebSerialLogger.println("Rotary encoder started");
}

int RotaryEncoder::GetCounter()
{
    return counter;
}

void RotaryEncoder::Loop()
{
    lastCounter = counter;

    // read X and Y analog values
    _currValueAB  = digitalRead(ENC_A) << 1;
    _currValueAB |= digitalRead(ENC_B);
    
    switch ((_prevValueAB | _currValueAB))
    {
                                                    //fast MCU
      //case 0b0001: case 0b1110:                                   //CW states, 1 count  per click
    case 0b0001: case 0b1110: case 0b1000: case 0b0111:         //CW states, 2 counts per click
        counter++;
        break;                                              //fast MCU
      //case 0b0100: case 0b1011:                                   //CCW states, 1 count  per click
    case 0b0100: case 0b1011: case 0b0010: case 0b1101:         //CCW states, 2 counts per click
        counter--;
        break;
    }

    _prevValueAB = _currValueAB << 2;

    int SW = digitalRead(_sw);
    if(SW == 0 && SW_OLD != 0)
      SwitchPressed = true;
    else
      SwitchPressed = false;

    SW_OLD = SW;

    if(counter != lastCounter)
    {
      WebSerialLogger.println("New encoder value: "  + String(counter));
      
    }
}