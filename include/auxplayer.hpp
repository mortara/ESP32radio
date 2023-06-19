#include <Arduino.h>

#ifndef AUXPLAYER_H
#define AUXPLAYER_H

class AuxPlayerClass
{

    public:
        String GetFreqDisplayText();
        String GetClockDisplayText();
};

extern AuxPlayerClass AuxPlayer;

#endif