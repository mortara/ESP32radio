#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

typedef struct {
    String SSID;  //stream url
    String PASS; //stations name
} WIFICreds;

class WIFIManager
{
    private:

        bool connected = false;
        WIFICreds _credentials = {WIFISSID, WIFIPASS};
        unsigned long previousMillis = 0;
        unsigned long interval = 15000;
        unsigned long _lastConnectionTry;
        TaskHandle_t Task1;

    public:

        WIFIManager();
        bool Connect();
        void Disconnect();
        void Loop(char ch);
        void DisplayInfo();
        bool IsConnected();
        unsigned long LastConnectionTry();
};