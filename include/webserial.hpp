
#include <Arduino.h>
#include <WiFi.h>
#include <WebSerial.h>

#ifndef WEBSERIAL_H
#define WEBSERIAL_H


class WebSerialLoggerClass
{
    private:
        AsyncWebServer *_server;
        bool running = false;

    public:
        WebSerialLoggerClass();
        void Begin(AsyncWebServer *_server);
        bool IsRunning();
        void print(const char *text);
        void print(String text = "");
        void println(const char *text);
        void println(String text = "");
        void Flush();
        char GetInput();
};
#endif

extern WebSerialLoggerClass WebSerialLogger;
