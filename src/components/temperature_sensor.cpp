#include "temperature_sensor.hpp"


void TemperatureSensorClass::Begin(uint8_t adr) 
{
    i2cdevice::Setup(&Wire, adr);

    if(!isActive())
    {
        pmLogging.LogLn("temperature sensor not found!");     
        return;   
    }

    pmLogging.LogLn("Initializing temperature sensor");

    if(_bmp.begin())
        _active = true;

    _lastRead = millis();
}

bool TemperatureSensorClass::mqttSetup()
{
    if(setupmqtt)
        return false;

    pmLogging.LogLn("Setting up MQTT client");

    if(!pmCommonLib.MQTTConnector.SetupSensor("Temperature", "BMP180", "temperature", "°C", "mdi:temperature-celsius"))
    {
        pmLogging.LogLn("Could not setup temperature sensor!");
        return false;
    }

    pmCommonLib.MQTTConnector.SetupSensor("Pressure", "BMP180", "pressure", "Pa", "mdi:air-filter");
    pmCommonLib.MQTTConnector.SetupSensor("Altitude", "BMP180", "", "m", "");
    setupmqtt = true;

    pmLogging.LogLn("Temperature Sensor mqtt setup done!");
    return true;
}

void TemperatureSensorClass::DisplayInfo()
{
    pmLogging.Log("Temperature = ");
    pmLogging.Log(String(_temperature));
    pmLogging.LogLn(" *C");
    pmLogging.Log("Pressure = ");
    pmLogging.Log(String(_pressure));
    pmLogging.LogLn(" Pa");
    pmLogging.Log("Altitude = ");
    pmLogging.Log(String(_altitude));
    pmLogging.LogLn("m");
}

float TemperatureSensorClass::GetLastTemperatureReading()
{
    return _temperature;
}

void TemperatureSensorClass::Loop() {

    if(!_active)
        return;

    unsigned long now = millis();
    if(now - _lastRead < 15000UL)
        return;

    _lastRead = now;

    if(pmCommonLib.MQTTConnector.isActive() && !setupmqtt)
        mqttSetup();

    _temperature = _bmp.readTemperature();
    _pressure = _bmp.readPressure();
    _altitude = _bmp.readAltitude();
    
    if(setupmqtt)
    {
        JsonDocument payload;
        payload["Temperature"] = String(_temperature);
        payload["Pressure"] = String(_pressure);
        payload["Altitude"] = String(_altitude);
        
        pmCommonLib.MQTTConnector.PublishMessage(payload, "BMP180");

    }
}

TemperatureSensorClass TemperatureSensor1;