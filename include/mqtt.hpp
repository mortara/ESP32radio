#include <Arduino.h>
#include "PubSubClient.h"
#include "WiFi.h"
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include "webserial.hpp"
#include <list>

#ifndef MQTTCONNECTOR_H
#define MQTTCONNECTOR_H

struct MQTTMessages
{
    String payload;
    String component;
    bool Retain = false;
};

class MQTTConnectorClass
{
    private:
        WiFiClient *_wifiClientmqtt = NULL;
        PubSubClient *_mqttClient;
        bool _active = false;
        String device_id = "esp32radio";
        unsigned long _lastConnectAttempt;
        unsigned long _lastMqTTLoop = 0;

        
    public:
        void Setup();
        void Loop();
        void PublishMessage(JsonDocument msg, String component, bool retain = false);
        void SendPayload(String msg, String component, bool retain = false);
        bool isActive();
        bool SetupSensor(String topic, String sensor, String component, String deviceclass = "", String unit = "", String icon = "");
        bool Connect();

        std::list<MQTTMessages *>* Tasks;
        volatile bool lock;   
        static void Task1code(void *pvParameters);
};

extern MQTTConnectorClass MQTTConnector;

#endif