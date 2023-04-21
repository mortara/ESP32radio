#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "radio.hpp"


Radio *_radio;
unsigned long _loopStart;
int _loopCount;

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
  Serial.begin(57600);
  //ScanI2C();
  _radio = new Radio();

}

// Main
void loop()
{
  _loopCount++;
  if(_loopCount == 5000)
  {
      unsigned long end = millis();
      float duration = (float)(end - _loopStart) / (float)5000.0;
      //Serial.print(duration);
      Serial.println("loop: " + String(duration) + "ms");
      _loopCount = 0;
      _loopStart = end;
  }

  
  _radio->Loop();
}


