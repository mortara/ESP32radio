#include <Arduino.h>
#include <DNSServer.h>
#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "displaydata.hpp"

#ifndef WEBSERVER_H
#define WEBSERVER_H

class WebServer
{
    private:
        
    public:
        WebServer();
        void Loop(const DisplayData &data);

} ;

#endif