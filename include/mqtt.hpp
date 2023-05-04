#include <Arduino.h>
#include "PubSubClient.h"
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

#ifndef MQTTCONNECTOR_H
#define MQTTCONNECTOR_H

class MQTTConnector
{
    private:
        WiFiClient *_wifiClientmqtt;
        PubSubClient *_mqttClient;
        
        String device_id = "esp32radio";
        String sensor_topic_head = "homeassistant/sensor/" + device_id;

    public:
        MQTTConnector();
        void Loop();
        void PublishSensor(String msg);
        bool SetupSensor(String topic, String unit, String icon, String entity_category="");
};

#endif