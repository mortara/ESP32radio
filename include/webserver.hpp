#include <Arduino.h>
#include <DNSServer.h>
#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "displaydata.hpp"

#ifndef WEBSERVER_H
#define WEBSERVER_H

class WebServerClass
{
    private:
        AsyncWebServer *_webserver;
        DNSServer dnsServer;
        DisplayData _displaydata;
    public:
        void Setup();
        AsyncWebServer *GetServer();

} ;

extern WebServerClass WebServer;

#endif