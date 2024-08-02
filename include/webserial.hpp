
#include <Arduino.h>
#include <WiFi.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
//#include <WebSerial.h>

#ifndef WEBSERIAL_H
#define WEBSERIAL_H


class WebSerialLoggerClass
{
    private:
        AsyncWebServer *_server;
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
        char GetInput();
};


extern WebSerialLoggerClass WebSerialLogger;
#endif