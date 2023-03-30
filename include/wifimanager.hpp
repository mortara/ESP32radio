#include <Arduino.h>
#include <WiFi.h>


typedef struct {
    String SSID;  //stream url
    String PASS; //stations name
} WIFICreds;

class WIFIManager
{
    private:

        bool connected = false;
        WIFICreds _credentials = {"WELAHN2G", "dukommsthiernichtrein"};
        unsigned long previousMillis = 0;
        unsigned long interval = 15000;
        TaskHandle_t Task1;

    public:

        void Start();
        void Connect();
        void Disconnect();
        void Loop(char ch);
        void DisplayInfo();
};