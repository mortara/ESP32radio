#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "radio.hpp"
#include "pmCommonLib.hpp"

I2CScanner *_i2cscanner;

unsigned long _lastLoop;

unsigned long _loopTimeMax = 0;
unsigned long _loopTimeMin = 9999999;
unsigned long _loopTimeSum = 0;
float _loopTimeAverage = 0;
int _loopTimeCount = 0;
int _loopsToCount = 1000;

int mode = 0;

bool crashed = false;


void ScanI2C()
{
  byte error = 0;
  byte address = 0;
  int nDevices = 0;
  TwoWire I2C_1 = TwoWire(0);
  I2C_1.begin(21,22, 40000);

  Serial.println("\r\nScanning for I2C devices ...");
  for(address = 0x01; address < 0x7f; address++){
    I2C_1.beginTransmission(address);
    error = I2C_1.endTransmission();
    if (error == 0){
      Serial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    }
  }

  I2C_1.end();

  TwoWire I2C_2 = TwoWire(1);
  I2C_2.begin(33,32, 10000);

  Serial.println("\r\nScanning for I2C-2 devices ...");
  for(address = 0x01; address < 0x7f; address++){
    I2C_2.beginTransmission(address);
    error = I2C_2.endTransmission();
    if (error == 0){
      Serial.printf("I2C-2 device found at address 0x%02X\n", address);
      nDevices++;
    }
  }

  I2C_2.end();

  if (nDevices == 0){
    Serial.println("No I2C devices found");
  }
}

void setup()
{
  setCpuFrequencyMhz(80);
  
  Serial.begin(115200);
  /*Serial0.begin(115200);
  Serial1.begin(115200);
  Serial2.begin(115200);*/
  
  //delay(100);
  //while(!Serial)
  //  delay(100);
  //Wire.begin(41,42);
  //Serial.println("Hello Serial!");
  //Serial.println("Serial Hello!");
  /*Serial0.println("Serial0 Hello!");
  Serial1.println("Serial1 Hello!");
  Serial2.println("Serial2 Hello!");*/
  pmLogging.LogLn("Hello world!");
  //ScanI2C();

  try
  {
     pmCommonLib.Setup();
     pmCommonLib.MQTTConnector.ConfigureDevice(DEVICE_NAME, "RS555", "Patrick Mortara");

      _radio.Setup();

      pmCommonLib.Start();

      if(_radio.OTAOnly)
        pmCommonLib.OTAHandler.OTAOnly = true;
  }
  catch(const std::exception& e)
  {
    crashed = true;
    pmCommonLib.WiFiManager.Connect();
  }
}

void DisplayLoopTime()
{
    unsigned long now = millis();
    unsigned long looptime = now - _lastLoop;
    _lastLoop = now;

    if(looptime > _loopTimeMax)
      _loopTimeMax = looptime;

    if(looptime < _loopTimeMin)
      _loopTimeMin = looptime;

    _loopTimeSum += looptime;
    _loopTimeCount++;

    if(_loopTimeCount == _loopsToCount)
    {
      _loopTimeAverage = (float)_loopTimeSum / (float)_loopTimeCount;
      Serial.printf("Looptime: Avg: %f   Min: %u  Max: %u \r\n", _loopTimeAverage, _loopTimeMin, _loopTimeMax);

      _loopTimeCount = 0;
      _loopTimeSum = 0;
      _loopTimeMax = 0;
      _loopTimeMin = 9999999;
    }

}


// Main
void loop()
{
  DisplayLoopTime();
   
  if(pmCommonLib.OTAHandler.OTAOnly)
    pmCommonLib.Loop();

  if(mode == 0)
  {
    try
    {
      char ch = _radio.Loop();

      switch(ch)
      {
        case 'D':
          _radio.Stop();
          ScanI2C();
          break;
        case 'S':
          _radio.Stop();

          _i2cscanner = new I2CScanner();
          _i2cscanner->setup();
          mode = 1;
          break;
        case 'L':
          Serial.printf("Looptime: Avg: %f   Min: %u  Max: %u \r\n", _loopTimeAverage, _loopTimeMin, _loopTimeMax);
          pmLogging.LogLn("loop: Avg. " + String(_loopTimeAverage));
          break;
      }
    }
    catch(const std::exception& e)
    {
      crashed = true;
    }
  } 
  else if(mode == 1)
  {
    _i2cscanner->loop();
  }

}


