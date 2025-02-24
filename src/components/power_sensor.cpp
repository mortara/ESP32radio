#include "power_sensor.hpp"

void PowerSensorClass::Begin(uint8_t adr)
{
    i2cdevice::Setup(&Wire, adr);

    WebSerialLogger.println("Initializing power sensor");
    if(!isActive())
    {
        WebSerialLogger.println("Power sensor not found!");     
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

    WebSerialLogger.println("Setting up MQTT client");

    if(!MQTTConnector.SetupSensor("Current", "sensor", "INA219", "current", "mA", "mdi:current-dc"))
    {
        WebSerialLogger.println("Could not setup current sensor!");
        return false;
    }

    MQTTConnector.SetupSensor("BusVoltage", "sensor", "INA219", "voltage", "V", "mdi:flash-triangle");
    MQTTConnector.SetupSensor("ShuntVoltage", "sensor", "INA219", "voltage", "mV", "mdi:flash-triangle");
    MQTTConnector.SetupSensor("LoadVoltage", "sensor", "INA219", "voltage", "V", "mdi:flash-triangle");
    MQTTConnector.SetupSensor("Power", "sensor", "INA219", "power", "W", "mdi:flash-triangle");

    setupmqtt = true;

    WebSerialLogger.println("Power Sensor mqtt setup done!");
    return true;
}

void PowerSensorClass::DisplayInfo()
{
    WebSerialLogger.print("Current = ");
    WebSerialLogger.print(String(_current));
    WebSerialLogger.println("mA");

    WebSerialLogger.print("BusVoltage = ");
    WebSerialLogger.print(String(_busvoltage));
    WebSerialLogger.println(" V");

    WebSerialLogger.print("ShuntVoltage = ");
    WebSerialLogger.print(String(_shuntvoltage));
    WebSerialLogger.println(" mV");

    WebSerialLogger.print("Power = ");
    WebSerialLogger.print(String(_power));
    WebSerialLogger.println(" mW");
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

    if(!setupmqtt && MQTTConnector.isActive())
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

        MQTTConnector.PublishMessage(payload, "INA219");
    }
}

PowerSensorClass PowerSensor;