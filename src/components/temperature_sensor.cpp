#include "temperature_sensor.hpp"


TemperatureSensor::TemperatureSensor(MQTTConnector *mqtt) : i2cdevice(0x77)
{
    if(!isActive())
    {
        WebSerialLogger.println("temperature sensor not found!");     
        return;   
    }

    WebSerialLogger.println("Initializing temperature sensor");
    _bmp = new Adafruit_BMP085();
    _bmp->begin();

    _lastRead = millis();

    _mqtt = mqtt;
}

bool TemperatureSensor::mqttSetup()
{
    if(setupmqtt)
        return false;

    WebSerialLogger.println("Setting up MQTT client");

    if(!_mqtt->SetupSensor("Temperature", "sensor", "BMP180", "temperature", "*C", "mdi:temperature-celsius"))
    {
        WebSerialLogger.println("Could not setup temperature sensor!");
        return false;
    }

    _mqtt->SetupSensor("Pressure", "sensor", "BMP180", "pressure", " Pa", "mdi:air-filter");
    setupmqtt = true;

    WebSerialLogger.println("Temperature Sensor mqtt setup done!");
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
    WebSerialLogger.print("Temperature = ");
    WebSerialLogger.print(String(_temperature));
    WebSerialLogger.println(" *C");

    float _pressure = _bmp->readPressure();
    WebSerialLogger.print("Pressure = ");
    WebSerialLogger.print(String(_pressure));
    WebSerialLogger.println(" Pa");
    
    if(setupmqtt)
    {
        String payload ="{ \"Temperature\": " + String(_temperature) + ", \"Pressure\": " + String(_pressure) + "}";
        _mqtt->PublishSensor(payload, "BMP180");
    }
}