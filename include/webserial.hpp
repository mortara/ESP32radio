
#include <Arduino.h>
#include <WiFi.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <MycilaWebSerial.h>

#ifndef WEBSERIAL_H
#define WEBSERIAL_H


class WebSerialLoggerClass
{
    private:
        
        WebSerial webSerial;
        bool running = false;

    public:
        void Start();
        void Begin(AsyncWebServer *_server);
        bool IsRunning();
        void print(const char *text);
        void print(String text = "");
        void println(const char *text);
        void println(String text = "");
        void Flush();
        void Loop();
        char GetInput();
};


extern WebSerialLoggerClass WebSerialLogger;
#endif