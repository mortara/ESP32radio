#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "radio.hpp"
#include "i2cscanner.hpp"
#include "ArduinoOTA.h"

Radio *_radio;
I2CScanner *_i2cscanner;
unsigned long _loopStart;
int _loopCount;
int _loopnum = 150000;
int mode = 0;
bool ota_running = false;
bool crashed = false;
unsigned long ota_timer = 0;

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

void start_ota()
{
  ArduinoOTA.onStart([]() {

    _radio->Stop();

    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    WebSerialLogger.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    WebSerialLogger.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    _radio->ShowPercentage(progress, total);
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("OTA started");
  ota_running = true;
  ota_timer = millis();
}

void setup()
{
  setCpuFrequencyMhz(80);
  
  Serial.begin(115200);
  Serial0.begin(115200);
  Serial1.begin(115200);
  Serial2.begin(115200);
  
  delay(100);
  //while(!Serial)
  //  delay(100);
  //Wire.begin(41,42);
  //Serial.println("Hello Serial!");
  Serial.println("Serial Hello!");
  Serial0.println("Serial0 Hello!");
  Serial1.println("Serial1 Hello!");
  Serial2.println("Serial2 Hello!");
  WebSerialLogger.println("Hello world!");
  //ScanI2C();

  try
  {
     _radio = new Radio();
      _radio->Setup();
  }
  catch(const std::exception& e)
  {
    crashed = true;
    WIFIManager.Connect();
  }
}

// Main
void loop()
{

    unsigned long now = millis();
    _loopCount++;
    if(_loopCount == _loopnum)
    {
        
        float duration = (float)(now - _loopStart) / (float)_loopnum;
        //Serial.print(duration);
        if(mode == 0)
          _radio->LoopTime  = duration;
          //WebSerialLogger.println("loop: " + String(duration) + "ms");
        _loopCount = 0;
        _loopStart = now;

        _loopnum = 100000;

        if(duration > 0.01)
          _loopnum = 50000;

        if(duration > 0.5)
          _loopnum = 25000;

        if(duration > 5)
          _loopnum = 10000;
    }

    if(now - ota_timer > 100UL)
    {
      if(crashed)
      {
        if(!WiFi.isConnected())
          WIFIManager.Connect();

      }

      if(ota_running)
        ArduinoOTA.handle();
      else
      {
        if(WiFi.isConnected())
          start_ota();
      }
      ota_timer = now;
    }

    if(mode == 0)
    {
      try
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
      }
      catch(const std::exception& e)
      {
        crashed = true;
      }
    } else if(mode == 1)
    {
      _i2cscanner->loop();
    }

}


