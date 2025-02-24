#include <Arduino.h>
#include "Webserial/webserial.hpp"

class Speaker
{
    private:
        int _relaispin = 0;
        bool _onoff = false;
    public:
        Speaker(int pin);

        void TurnOn();
        void TurnOff();
        void Toggle();
        
        void ExecuteCommand(char ch);
};