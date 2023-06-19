#include <Arduino.h>
#include "temperature_sensor.hpp"
#include "power_sensor.hpp"

#ifndef MENU_H
#define MENU_H

class MenuClass
{

    public:
        String GetLine(int page, int row);
        int InfoPages = 3;
};


extern MenuClass Menu;

#endif