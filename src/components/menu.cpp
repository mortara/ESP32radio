#include "menu.hpp"

String MenuClass::GetLine(int page, int row)
{
    switch(page)
    {
        case -1:
            return "Temp.: " + String(TemperatureSensor1.GetLastTemperatureReading(),1) + " C";
            break;
        case -2:
            return "Curr.: " + String(PowerSensor.GetLastCurrentReading(),1) + " mA";
            break;
        case -3:
            if(WiFi.isConnected())
                return "WIFI connected!";
            else    
                return "No WIFI!";
            break;
    }

    return "";
}

MenuClass Menu;