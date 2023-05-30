#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "radio.hpp"
#include "i2cscanner.hpp"

Radio *_radio;
I2CScanner *_i2cscanner;
unsigned long _loopStart;
int _loopCount;
int _loopnum = 150000;
int mode = 0;

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
  Serial.begin(115200);
  WebSerialLogger.println("Hello world!");
  //ScanI2C();
  _radio = new Radio();

}

// Main
void loop()
{
    _loopCount++;
    if(_loopCount == _loopnum)
    {
        unsigned long end = millis();
        float duration = (float)(end - _loopStart) / (float)_loopnum;
        //Serial.print(duration);
        if(mode == 0)
          WebSerialLogger.println("loop: " + String(duration) + "ms");
        _loopCount = 0;
        _loopStart = end;

        _loopnum = 100000;

        if(duration > 0.01)
          _loopnum = 50000;

        if(duration > 0.5)
          _loopnum = 25000;

        if(duration > 5)
          _loopnum = 10000;
    }

    if(mode == 0)
    {
      char ch = _radio->Loop();
      switch(ch)
      {
        case 'D':
          _radio->Stop();
          ScanI2C();
          break;
        case 'S':
          _radio->Stop();

          _i2cscanner = new I2CScanner();
          _i2cscanner->setup();
          mode = 1;
          break;
        case 'L':
          WebSerialLogger.println("loop: " + String(_loopCount));
          break;
      }
    } else if(mode == 1)
    {
      _i2cscanner->loop();
    }
}


