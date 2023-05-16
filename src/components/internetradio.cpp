#include "internetradio.hpp"
#include "mqtt.hpp"

#define MP3buffersize 32

InternetRadio::InternetRadio(VS1053Player *player, DACIndicator *freq, DACIndicator *signal)
{
    WebSerialLogger.println("InternetRadio setup ...");
    _mp3buff = new uint8_t[MP3buffersize];
    _player = player;

    _pwmindicator_freq = freq;
    _pwmindicator_signal = signal;

    _signaltimer = millis();
}

void InternetRadio::setupMQTT()
{
    if(mqttsetup)
        return;

    WebSerialLogger.println("Setting up Internetradio MQTT client");

    MQTTConnector.SetupSensor("Station", "sensor", "Internetradio", "", "", "");
    MQTTConnector.SetupSensor("BytesPlayed", "sensor", "Internetradio", "", "", "mdi:radio");

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
    Serial.print("connecting to ");
    Serial.println(station.url);

    if(_http != NULL)
    {
        _http->end();
        delete _http;
    }

    _http = new HTTPClient();
    _http->setConnectTimeout(5000);
    _http->setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    if(_client != NULL)
    {
        _client->stop();
        delete _client;
        _client = NULL;
    }
    bytes_served = 0;
    // Your Domain name with URL path or IP address with path
    _http->begin(station.url);
    
    // Send HTTP GET request
    int httpResponseCode = _http->GET();
    
    WebSerialLogger.print("return code: ");
    WebSerialLogger.println(String(httpResponseCode));

    if (httpResponseCode>0) {
        if(httpResponseCode == HTTP_CODE_OK) {
                // get tcp stream
                _client = _http->getStreamPtr();

                if(_client->connected())
                    WebSerialLogger.println("Stream started!");
        }
    }
}

void InternetRadio::Start()
{
    WebSerialLogger.println("InternetRadio start ...");
    _pwmindicator_signal->SetRange(0, 120);
    _player->Begin();
    StartStream(stationlist[_current_station_preset]);
}

void InternetRadio::Stop()
{
    WebSerialLogger.println("InternetRadio stop ...");
    _http->end();
    _player->End();
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

            DynamicJsonDocument payload(2048);
            payload["Station"] = String(st.name);
            payload["BytesPlayed"] = String(bytes_served);

            String state_payload  = "";
            serializeJson(payload, state_payload);
            
            MQTTConnector.PublishMessage(state_payload, "Internetradio");
        }
    }

    if(now - _signaltimer > 1000)
    {
        _pwmindicator_signal->SetValue((uint16_t)abs(WiFi.RSSI()));
        _signaltimer = now;
    }

    if(_client != NULL && _client->connected())
    {
        if (_player->ReadyForData()) 
        {
            if (_client->available() >= MP3buffersize) {
                //Serial.println("playing data");
                // The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
                uint8_t bytesread = _client->read(_mp3buff, MP3buffersize);
                bytes_served += bytesread;
                if(bytesread > 0)
                {
                    _player->PlayData(_mp3buff, bytesread);
                    /*if(bytesread != MP3buffersize)
                        WebSerialLogger.println("Data buffer != " + String(MP3buffersize) + " bytes");*/
                }
                else
                    WebSerialLogger.println("no data in buffer");

            }
        }  

    } 
    else if(WiFi.isConnected())
    {
        WebSerialLogger.println("Try to connect ...");
        StartStream(stationlist[_current_station_preset]);
    }

}