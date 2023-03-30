#include <Arduino.h>


class Speaker
{
    private:
        int _relaispin = 0;
        bool _onoff = false;
    public:
        Speaker(int pin);

        void TurnOn();
        void TurnOff();

        void ExecuteCommand(char ch);
};