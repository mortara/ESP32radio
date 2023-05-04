#include <Arduino.h>
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include "mqtt.hpp"

class TemperatureSensor
{
    private:
        Adafruit_BMP085 *_bmp;
        MQTTConnector *_mqtt;
        unsigned long _lastRead;
        bool setupmqtt = false;

        void mqttSetup();

    public:
        TemperatureSensor(MQTTConnector *mqtt);
        void Loop();

};