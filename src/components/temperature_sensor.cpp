#include "temperature_sensor.hpp"


TemperatureSensor::TemperatureSensor(MQTTConnector *mqtt)
{
    Serial.println("Initializing temperature sensor");
    _bmp = new Adafruit_BMP085();
    _lastRead = millis();

    _mqtt = mqtt;
}

void TemperatureSensor::mqttSetup()
{
    if(setupmqtt)
        return;

    Serial.println("Setting up MQTT client");

    if(!_mqtt->SetupSensor("Temperature", "*C", "mdi:temperature-celsius", "diagnostic"))
    {
        Serial.println("Could not setup temperature sensor!");
        return;
    }

    _mqtt->SetupSensor("Pressure", " Pa", "", "diagnostic");
    setupmqtt = true;
}

void TemperatureSensor::Loop() {

    unsigned long now = millis();
    if(now - _lastRead < 10000)
        return;

    if(WiFi.status() == WL_CONNECTED && !setupmqtt)
        mqttSetup();

    float _temperature = _bmp->readTemperature();
    Serial.print("Temperature = ");
    Serial.print(_temperature);
    Serial.println(" *C");

    float _pressure = _bmp->readPressure();
    Serial.print("Pressure = ");
    Serial.print(_pressure);
    Serial.println(" Pa");
    
    String payload ="{ 'Temperature': " + String(_temperature) + ", 'Pressure': " + String(_pressure) + "}";
    _mqtt->PublishSensor(payload);
}