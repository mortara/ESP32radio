#include "ota_handler.h"

void ota_handler::Start()
{
    ArduinoOTA.onStart(std::bind(&ota_handler::onStart, this));

    ArduinoOTA.onEnd(std::bind(&ota_handler::onEnd, this));

    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) { this->onProgress(progress, total); });

    ArduinoOTA.onError([this](ota_error_t error) { this->onError(error); });
      
    ArduinoOTA.begin();

    WebSerialLogger.println("OTA started");
    ota_running = true;
    ota_timer = millis();
}

void ota_handler::onStart()
{
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) 
    {
        type = "sketch";
    } else {  // U_FS
        type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    WebSerialLogger.println("Start updating " + type);
    OTAOnly = true;
}

void ota_handler::onEnd()
{
    WebSerialLogger.println("Done!");
    WebSerialLogger.println("Rebooting");

    delay(1000);
}

void ota_handler::onProgress(unsigned int progress, unsigned int total)
{
    int perc = (progress / (total / 100));

    if(perc != last_perc)
    {
        String str = "Progress: " + String(perc) + "\r";
        WebSerialLogger.println(str.c_str());
        last_perc = perc;
    }
}

void ota_handler::onError(ota_error_t error)
{
    Serial.printf("Error[%u]: ", error);

    if (error == OTA_AUTH_ERROR) {
        WebSerialLogger.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
        WebSerialLogger.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
        WebSerialLogger.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
        WebSerialLogger.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
        WebSerialLogger.println("End Failed");
    }
}

void ota_handler::Loop()
{
    unsigned long now = millis();
   
    if(now - ota_timer > 100UL)
    {
      if(ota_running)
        ArduinoOTA.handle();
      else
      {
        if(WiFi.isConnected())
          Start();
      }
      ota_timer = now;
    }
}