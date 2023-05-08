#include "power_sensor.hpp"


PowerSensor::PowerSensor(uint8_t adr, MQTTConnector *mqtt) : i2cdevice(adr)
{
    if(!isActive())
    {
        WebSerialLogger.println("Power sensor not found!");     
        return;   
    }

    WebSerialLogger.println("Initializing power sensor");
    _ina = new Adafruit_INA219();
    _ina->begin();
    _ina->setCalibration_32V_1A();
    _lastRead = millis();

    _mqtt = mqtt;
}

bool PowerSensor::mqttSetup()
{
    if(setupmqtt)
        return false;

    WebSerialLogger.println("Setting up MQTT client");

    if(!_mqtt->SetupSensor("Current", "sensor", "INA219", "current", "mA", "mdi:current-dc"))
    {
        WebSerialLogger.println("Could not setup current sensor!");
        return false;
    }

    _mqtt->SetupSensor("BusVoltage", "sensor", "INA219", "voltage", "V", "mdi:flash-triangle");
    _mqtt->SetupSensor("ShuntVoltage", "sensor", "INA219", "voltage", "mV", "mdi:flash-triangle");
    _mqtt->SetupSensor("LoadVoltage", "sensor", "INA219", "voltage", "V", "mdi:flash-triangle");

    _mqtt->SetupSensor("Power", "sensor", "INA219", "power", "mW", "mdi:flash-triangle");

    setupmqtt = true;

    WebSerialLogger.println("Power Sensor mqtt setup done!");
    return true;
}

void PowerSensor::Loop(char ch) {

    if(_ina == NULL)
        return;

    unsigned long now = millis();
    if(now - _lastRead < 5000)
        return;

    _lastRead = now;

    if(_mqtt->isActive() && !setupmqtt)
        mqttSetup();

    float _current = _ina->getCurrent_mA();
    float _busvoltage = _ina->getBusVoltage_V();
    float _shuntvoltage = _ina->getShuntVoltage_mV();
    float _loadvoltage = _busvoltage + (_shuntvoltage / 1000);
    float _power = _ina->getPower_mW();

    if(ch == 'j')
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
    
    if(setupmqtt)
    {
        String payload ="{ \"Current\": " + String(_current) + ", \"BusVoltage\": " + String(_busvoltage) + ", \"ShuntVoltage\": " + String(_shuntvoltage) + ", \"LoadVoltage\": " + String(_loadvoltage) + ", \"Power\": " + String(_power) + "}";
        _mqtt->PublishSensor(payload, "INA219");
    }
}