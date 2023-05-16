#include "temperature_sensor.hpp"
#include "mqtt.hpp"

TemperatureSensor::TemperatureSensor(uint8_t adr) : i2cdevice(adr)
{
    if(!isActive())
    {
        WebSerialLogger.println("temperature sensor not found!");     
        return;   
    }

    WebSerialLogger.println("Initializing temperature sensor");

    if(_bmp.begin())
        _active = true;

    _lastRead = millis();
}

bool TemperatureSensor::mqttSetup()
{
    if(setupmqtt)
        return false;

    WebSerialLogger.println("Setting up MQTT client");

    if(!MQTTConnector.SetupSensor("Temperature", "sensor", "BMP180", "temperature", "*C", "mdi:temperature-celsius"))
    {
        WebSerialLogger.println("Could not setup temperature sensor!");
        return false;
    }

    MQTTConnector.SetupSensor("Pressure", "sensor", "BMP180", "pressure", " Pa", "mdi:air-filter");
    MQTTConnector.SetupSensor("Altitude", "sensor", "BMP180", "", "m", "");
    setupmqtt = true;

    WebSerialLogger.println("Temperature Sensor mqtt setup done!");
    return true;
}

void TemperatureSensor::DisplayInfo()
{
    WebSerialLogger.print("Temperature = ");
    WebSerialLogger.print(String(_temperature));
    WebSerialLogger.println(" *C");
    WebSerialLogger.print("Pressure = ");
    WebSerialLogger.print(String(_pressure));
    WebSerialLogger.println(" Pa");
    WebSerialLogger.print("Altitude = ");
    WebSerialLogger.print(String(_altitude));
    WebSerialLogger.println("m");
}

void TemperatureSensor::Loop() {

    if(!_active)
        return;

    unsigned long now = millis();
    if(now - _lastRead < 15000)
        return;

    _lastRead = now;

    if(MQTTConnector.isActive() && !setupmqtt)
        mqttSetup();

    _temperature = _bmp.readTemperature();
    _pressure = _bmp.readPressure();
    _altitude = _bmp.readAltitude();

    if(setupmqtt)
    {
        DynamicJsonDocument payload(2048);
        payload["Temperature"] = String(_temperature);
        payload["Pressure"] = String(_pressure);
        payload["Altitude"] = String(_altitude);
        
        String state_payload  = "";
        serializeJson(payload, state_payload);
        
        MQTTConnector.PublishMessage(state_payload, "BMP180");

    }
}