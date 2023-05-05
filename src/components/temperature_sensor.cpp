#include "temperature_sensor.hpp"


TemperatureSensor::TemperatureSensor(MQTTConnector *mqtt) : i2cdevice(0x77)
{
    if(!isActive())
    {
        Serial.println("temperature sensor not found!");     
        return;   
    }

    Serial.println("Initializing temperature sensor");
    _bmp = new Adafruit_BMP085();
    _bmp->begin();

    _lastRead = millis();

    _mqtt = mqtt;
}

bool TemperatureSensor::mqttSetup()
{
    if(setupmqtt)
        return false;

    Serial.println("Setting up MQTT client");

    if(!_mqtt->SetupSensor("Temperature", "sensor", "BMP180", "temperature", "*C", "mdi:temperature-celsius"))
    {
        Serial.println("Could not setup temperature sensor!");
        return false;
    }

    _mqtt->SetupSensor("Pressure", "sensor", "BMP180", "pressure", " Pa", "mdi:air-filter");
    setupmqtt = true;

    Serial.println("Temperature Sensor mqtt setup done!");
    return true;
}

void TemperatureSensor::Loop() {

    if(_bmp == NULL)
        return;

    unsigned long now = millis();
    if(now - _lastRead < 15000)
        return;

    _lastRead = now;

    if(_mqtt->isActive() && !setupmqtt)
        mqttSetup();

    float _temperature = _bmp->readTemperature();
    Serial.print("Temperature = ");
    Serial.print(String(_temperature));
    Serial.println(" *C");

    float _pressure = _bmp->readPressure();
    Serial.print("Pressure = ");
    Serial.print(String(_pressure));
    Serial.println(" Pa");
    
    if(setupmqtt)
    {
        String payload ="{ \"Temperature\": " + String(_temperature) + ", \"Pressure\": " + String(_pressure) + "}";
        _mqtt->PublishSensor(payload, "BMP180");
    }
}