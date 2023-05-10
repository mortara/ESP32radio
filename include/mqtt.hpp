#include <Arduino.h>
#include "PubSubClient.h"
#include "WiFi.h"
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include "webserial.hpp"

#ifndef MQTTCONNECTOR_H
#define MQTTCONNECTOR_H

class MQTTConnectorClass
{
    private:
        WiFiClient *_wifiClientmqtt;
        PubSubClient *_mqttClient;
        bool _active = false;
        String device_id = "esp32radio";
        unsigned long _lastConnectAttempt;

    public:
        MQTTConnectorClass();
        void Loop();
        void PublishSensor(String msg, String component);
        bool isActive();
        bool SetupSensor(String topic, String sensor, String component, String deviceclass = "", String unit = "", String icon = "", String entity_category="config");
};

extern MQTTConnectorClass MQTTConnector;

#endif