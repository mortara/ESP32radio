#include "mqtt.hpp"

void callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (byte i = 0; i < length; i++) {
        char tmp = char(payload[i]);
        msg += tmp;
    }
    WebSerialLogger.println(msg);
}

MQTTConnectorClass::MQTTConnectorClass()
{
    WebSerialLogger.println("Initializing MQTT client");

    _wifiClientmqtt = new WiFiClient();

    _mqttClient = new PubSubClient(MQTTBROKER, 1883, *_wifiClientmqtt);
    _mqttClient->setBufferSize(4096);
    _mqttClient->setCallback(callback);

    Tasks = new std::list<MQTTMessages *>();
    TaskHandle_t xHandle = NULL;

    xTaskCreatePinnedToCore(
        Task1code, /* Function to implement the task */
        "Task1", /* Name of the task */
        10000,  /* Stack size in words */
        this,  /* Task input parameter */
        0,  /* Priority of the task */
        &xHandle,  /* Task handle. */
        0); /* Core where the task should run */

    _lastConnectAttempt = millis();
}

bool MQTTConnectorClass::isActive()
{
    if(WiFi.status() != WL_CONNECTED)
        _active = false;

    return _active;
}

void MQTTConnectorClass::Loop()
{
    unsigned long now = millis();

    if(!_active && WiFi.status() == WL_CONNECTED && now - _lastConnectAttempt > 5000UL)
    {
        Connect();
    }

    if(now - _lastMqTTLoop > 4000UL)
    {
        _lastMqTTLoop = now;
        _mqttClient->loop();
    }
}

bool MQTTConnectorClass::Connect()
{
    WebSerialLogger.println("Connecting mqtt client ...");

    if(_wifiClientmqtt == NULL)
    {
        _wifiClientmqtt = new WiFiClient();
        _mqttClient->setClient(*_wifiClientmqtt);
    }

    _lastConnectAttempt = millis();
    if(!_mqttClient->connect(device_id.c_str(), MQTTUSER, MQTTPASSWORD))
    {
        WebSerialLogger.println("Could not connect to MQTT broker!");
        WebSerialLogger.println("State: " + String(_mqttClient->state()));
    }
    else
    {
        _mqttClient->subscribe("motd");
        _active = true;
    }

    return _active;
}

bool MQTTConnectorClass::SetupSensor(String topic, String sensor, String component, String deviceclass, String unit, String icon, String entity_category)
{
    if(!_active)
        return false;

    WebSerialLogger.println("Configuring sensor "+ topic);
    String sensor_topic_head = "homeassistant/" + sensor + "/" + device_id;

    String config_topic = sensor_topic_head + "_" + topic + "/config";
	String name = device_id + "_" + topic;

    DynamicJsonDocument root(2048);

    if(deviceclass != "")
        root["device_class"] = deviceclass;

    root["name"] = name;

    if(icon != "")
        root["icon"] = icon;

    if(unit != "")
        root["unit_of_measurement"] = unit;

    root["value_template"] = "{{ value_json." + topic + "}}";
    root["uniq_id"] = name;
    root["state_topic"] = "homeassistant/sensor/" + device_id + "_" + component + "/state";

    if(entity_category != "")
        root["entity_category"] = entity_category;

    JsonObject devobj = root.createNestedObject("dev");
    JsonArray deviceids = devobj.createNestedArray("ids");
    deviceids.add(device_id);

    devobj["name"] = device_id;
    devobj["mf"] = "Patrick Mortara";
    devobj["mdl"] = "ESP32Radio";

    String config_payload  = "";
    serializeJson(root, config_payload);

    WebSerialLogger.println("Topic: " + config_topic);

    //WebSerialLogger.println("Payload: " + config_payload);
    bool result = _mqttClient->publish(config_topic.c_str(), config_payload.c_str(), true);
    if(!result)
    {
        WebSerialLogger.println(" ... error!");
        WebSerialLogger.println("State: " + String(_mqttClient->state()));
    }
    _mqttClient->loop();
    return result;
}

void MQTTConnectorClass::SendPayload(String payload, String component, bool retain)
{
    if(!_active)
        return;
    String topic = "homeassistant/sensor/" + device_id + "_" + component + "/state";
    if(!_mqttClient->publish(topic.c_str(), payload.c_str(), retain))
    {
        WebSerialLogger.println("Error publishing data!");
        WebSerialLogger.println("State: " + String(_mqttClient->state()));

        if(_mqttClient->state() == -3)
        {
            WebSerialLogger.println("MQTT connection lost ...");
            _mqttClient->disconnect();
            _active = false;
        }
    }
}

void MQTTConnectorClass::PublishMessage(String msg, String component, bool retain)
{
    MQTTMessages *bt = new MQTTMessages();
    bt->payload = msg;
    bt->component = component;
    bt->Retain = retain;

    while(lock)
        delay(1);

    lock = true;
    Tasks->push_back(bt);
    lock = false;
}

void MQTTConnectorClass::Task1code(void *pvParameters) 
{
    // Add a delay to give the rest of the radio some time to setup
    delay(15000);

    for(;;) {
        if(MQTTConnector.Tasks->empty() || !MQTTConnector.isActive())
        {
            delay(1000);
        }
        else
        {
            while(MQTTConnector.lock)
                delay(1);

            MQTTConnector.lock = true;
            MQTTMessages *bt = MQTTConnector.Tasks->front();
            MQTTConnector.lock = false;
            
            MQTTConnector.SendPayload(bt->payload, bt->component);

            MQTTConnector.lock = true;
            MQTTConnector.Tasks->remove(bt);
            MQTTConnector.lock = false;
            delete bt;
        }
        }
    }

MQTTConnectorClass MQTTConnector;