#include "mqtt.hpp"

void callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (byte i = 0; i < length; i++) {
        char tmp = char(payload[i]);
        msg += tmp;
    }
    Serial.println(msg);
}

MQTTConnector::MQTTConnector()
{
    Serial.println("Initializing MQTT client");

    _wifiClientmqtt = new WiFiClient();

    _mqttClient = new PubSubClient(MQTTBROKER, 1883, *_wifiClientmqtt);
    _mqttClient->setCallback(callback);

    if(!_mqttClient->connect(device_id.c_str(), MQTTUSER, MQTTPASSWORD))
    {
        Serial.println("Could not connect to MQTT broker!");
        Serial.println("State: " + String(_mqttClient->state()));
    }
    else
    {
        _mqttClient->subscribe("motd");
    }
}

void MQTTConnector::Loop()
{
    _mqttClient->loop();
}

bool MQTTConnector::SetupSensor(String topic, String unit, String icon, String entity_category)
{
    Serial.println("Configuring sensor "+ topic);

    String config_topic = sensor_topic_head + "_" + topic + "/config";
	String name = device_id + "_" + topic;

    DynamicJsonDocument root(400);
    root["name"] = name;
    root["icon"] = icon;
    root["unit_of_measurement"] = unit;
    root["value_template"] = "{{ value_json." + topic + "}}";
    root["uniq_id"] = name;
    root["state_topic"] = sensor_topic_head + "/state";

    if(entity_category != "")
        root["entity_category"] = entity_category;

    JsonObject devobj = root.createNestedObject("dev");
    JsonArray deviceids = devobj.createNestedArray("ids");
    deviceids.add(device_id);

    String config_payload  = "";
    serializeJson(root, config_payload);

    Serial.println("Topic: " + config_topic);
    //serializeJsonPretty(root, Serial);

    Serial.println("Payload: " + config_payload);
    bool result = _mqttClient->publish_P(config_topic.c_str(), config_payload.c_str(), true);
    if(!result)
    {
        Serial.println(" ... error!");
        Serial.println("State: " + String(_mqttClient->state()));
        //delay(5000);
    }

    //String subs = device_id + "/" + topic;

        //client.subscribe(subs.c_str());

    return result;
}

void MQTTConnector::PublishSensor(String payload)
{
    String topic = sensor_topic_head  + "/state";
    if(!_mqttClient->publish(topic.c_str(), payload.c_str()))
    {
      Serial.println("Error publishing data!");
      Serial.println("State: " + String(_mqttClient->state()));
    }
}