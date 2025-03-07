#include "power_sensor.hpp"

void PowerSensorClass::Begin(uint8_t adr)
{
    i2cdevice::Setup(&Wire, adr);

    pmLogging.LogLn("Initializing power sensor");
    if(!isActive())
    {
        pmLogging.LogLn("Power sensor not found!");     
        return;   
    }

    if(_ina.begin())
    {
        _ina.setCalibration_16V_400mA();
        _active = true;
    }
}

bool PowerSensorClass::mqttSetup()
{
    if(setupmqtt)
        return false;

    pmLogging.LogLn("Setting up MQTT client");

    if(!pmCommonLib.MQTTConnector.SetupSensor("Current", "INA219", "current", "mA", "mdi:current-dc"))
    {
        pmLogging.LogLn("Could not setup current sensor!");
        return false;
    }

    pmCommonLib.MQTTConnector.SetupSensor("BusVoltage", "INA219", "voltage", "V", "mdi:flash-triangle");
    pmCommonLib.MQTTConnector.SetupSensor("ShuntVoltage", "INA219", "voltage", "mV", "mdi:flash-triangle");
    pmCommonLib.MQTTConnector.SetupSensor("LoadVoltage", "INA219", "voltage", "V", "mdi:flash-triangle");
    pmCommonLib.MQTTConnector.SetupSensor("Power", "INA219", "power", "W", "mdi:flash-triangle");

    setupmqtt = true;

    pmLogging.LogLn("Power Sensor mqtt setup done!");
    return true;
}

void PowerSensorClass::DisplayInfo()
{
    pmLogging.Log("Current = ");
    pmLogging.Log(String(_current));
    pmLogging.LogLn("mA");

    pmLogging.Log("BusVoltage = ");
    pmLogging.Log(String(_busvoltage));
    pmLogging.LogLn(" V");

    pmLogging.Log("ShuntVoltage = ");
    pmLogging.Log(String(_shuntvoltage));
    pmLogging.LogLn(" mV");

    pmLogging.Log("Power = ");
    pmLogging.Log(String(_power));
    pmLogging.LogLn(" mW");
}

float PowerSensorClass::GetLastCurrentReading()
{
    return _current;
}

void PowerSensorClass::Loop() {

    if(!_active)
        return;

    unsigned long now = millis();
    if(now - _lastRead < 10000UL)
        return;

    _lastRead = now;

    if(!setupmqtt && pmCommonLib.MQTTConnector.isActive())
        mqttSetup();

    _current = _ina.getCurrent_mA();
    _busvoltage = _ina.getBusVoltage_V();
    _shuntvoltage = _ina.getShuntVoltage_mV();
    _loadvoltage = _busvoltage + (_shuntvoltage / 1000.0);
    _power = _ina.getPower_mW() / 1000.0;

    if(setupmqtt)
    {
        JsonDocument payload;
        payload["Current"] = String(_current);
        payload["BusVoltage"] = String(_busvoltage);
        payload["ShuntVoltage"] = String(_shuntvoltage);
        payload["LoadVoltage"] = String(_loadvoltage);
        payload["Power"] = String(_power);

        pmCommonLib.MQTTConnector.PublishMessage(payload, "INA219");
    }
}

PowerSensorClass PowerSensor;