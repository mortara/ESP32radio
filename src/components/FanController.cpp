#include "FanController.hpp"

void FanController::Begin(uint8_t pin)
{
    pwmpin = pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    WebSerialLogger.println("PWM fan sucessfully initialized.");
}

void FanController::StartFan()
{
    if(pwmpin == 0)
        return;

    WebSerialLogger.println("Start fan ...");
    digitalWrite(pwmpin, HIGH);
    FanState = true;
}

void FanController::StopFan()
{
    if(pwmpin == 0)
        return;

    WebSerialLogger.println("Stop fan ...");
    digitalWrite(pwmpin, LOW);
    FanState = false;
}