#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"
#include "webserial.hpp"

#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

typedef struct {
    String SSID;  //stream url
    String PASS; //stations name
} WIFICreds;

class WIFIManagerClass
{
    private:
        bool connecting = false;
        bool connected = false;
        WIFICreds _credentials = {WIFISSID, WIFIPASS};
        unsigned long interval = 15000;
        unsigned long _lastConnectionTry = 0;
        unsigned long _lastMqttupdate = 0;
        TaskHandle_t Task1;
        void setupMQTT();
        bool mqttsetup = false;

    public:

        void StartUp();
        bool Connect();
        void Disconnect();
        void Loop();
        void DisplayInfo();
        bool IsConnected();
        unsigned long LastConnectionTry();
};

extern WIFIManagerClass WIFIManager;

#endif