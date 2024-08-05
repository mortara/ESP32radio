#include "internetradio.hpp"
#include "mqtt.hpp"



InternetRadio::InternetRadio()
{
    WebSerialLogger.println("InternetRadio setup ...");
    
     _http.setConnectTimeout(5000);
    _http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    _signaltimer = millis();
    clockdisplaypagetimer = _signaltimer;
}

void InternetRadio::setupMQTT()
{
    if(mqttsetup)
        return;

    MQTTConnector.SetupSensor("Station", "sensor", "Internetradio", "text", "", "");
    MQTTConnector.SetupSensor("BytesPlayed", "sensor", "Internetradio", "data_size", "B", "mdi:radio");

    WebSerialLogger.println("Internetradio Sensor mqtt setup done!");

    mqttsetup = true;
}

void InternetRadio::SwitchPreset(uint8_t num)
{
    WebSerialLogger.println("InternetRadio::SwitchPreset to " + String(num));
    _current_station_preset = num;
    StartStream(stationlist[_current_station_preset]);
}

void InternetRadio::StartStream(Station station)
{
    WebSerialLogger.println("connecting to " + String(station.url));
   
    _http.end();
    delay(100);
    bytes_served = 0;
    // Your Domain name with URL path or IP address with path
    _http.begin(station.url);
    
    // Send HTTP GET request
    int httpResponseCode = _http.GET();
    
    WebSerialLogger.print("return code: ");
    WebSerialLogger.println(String(httpResponseCode));

    if (httpResponseCode>0) {
        if(httpResponseCode == HTTP_CODE_OK) {
                if(_http.getStreamPtr() != nullptr)
                    WebSerialLogger.println("Stream started!");
        }
    }
}

void InternetRadio::Start(uint8_t preset)
{
    WebSerialLogger.println("InternetRadio start ...");
    SignalIndicator.SetRange(0, 120);
    MP3Player.Begin();

    _current_station_preset = preset;

    StartStream(stationlist[_current_station_preset]);
}

void InternetRadio::Stop()
{
    WebSerialLogger.println("InternetRadio stop ...");
    _http.end();
    MP3Player.End();
}

void InternetRadio::DisplayInfo()
{
    WebSerialLogger.println("Current stream: " + String(stationlist[_current_station_preset].name));
    WebSerialLogger.println("Played " + String(bytes_served) + " bytes");
}

String InternetRadio::GetFreqDisplayText()
{
    Station st = stationlist[_current_station_preset];
    return String(st.name);
}

String InternetRadio::GetClockDisplayText()
{
    if(clockdisplaypage == 0)
    {
        Station st = stationlist[_current_station_preset];
        return String(st.name);
    }

    if(clockdisplaypage == 1)
    {
        
        return WiFi.SSID();
    }

    if(clockdisplaypage == 2)
    {
        
        return WiFi.localIP().toString();
    }

    if(clockdisplaypage == 3)
    {
        
        return (" RSSI: " + String(WiFi.RSSI()) + " dB");
    }

    return "";
}

void InternetRadio::Loop()
{
    unsigned long now = millis();
    if((now - clockdisplaypagetimer) > 15000)
    {
        clockdisplaypage++;
        if(clockdisplaypage == 4)
            clockdisplaypage = 0;
        
        clockdisplaypagetimer = now;

        if(MQTTConnector.isActive() && !mqttsetup)
            setupMQTT();

        if(mqttsetup)
        {   
            Station st = stationlist[_current_station_preset];

            JsonDocument payload;
            payload["Station"] = String(st.name);
            payload["BytesPlayed"] = bytes_served;

            //WebSerialLogger.println("Sending internetradio mqtt: " + String(st.name) + "/"+ String(bytes_served));

            MQTTConnector.PublishMessage(payload, "Internetradio");
        }
    }

    if(now - _signaltimer > 1000)
    {
        SignalIndicator.SetValue((uint16_t)abs(WiFi.RSSI()));
        _signaltimer = now;
    }

    WiFiClient *_client = _http.getStreamPtr();
    if(_client != nullptr && _client->connected())
    {
        if (MP3Player.ReadyForData()) 
        {
            if (_client->available() >= MP3buffersize) {
                //Serial.println("playing data");
                // The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
                uint8_t bytesread = _client->read(MP3Player.Mp3buffer, MP3buffersize);
                bytes_served += bytesread;
                if(bytesread > 0)
                {
                    MP3Player.PlayData(bytesread);
                    /*if(bytesread != MP3buffersize)
                        WebSerialLogger.println("Data buffer != " + String(MP3buffersize) + " bytes");*/
                }
                else
                    WebSerialLogger.println("no data in buffer");

            }
        }  

    } 
    else if(WiFi.status() == WL_CONNECTED )
    {
        WebSerialLogger.println("Try to connect ...");
        StartStream(stationlist[_current_station_preset]);
    }

}