#include "internetradio.hpp"
#include "mqtt.hpp"



InternetRadio::InternetRadio()
{
    WebSerialLogger.println("InternetRadio setup ...");
    
     _http.setConnectTimeout(5000);
    _http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    Stations = new std::list<Station *>();

    _signaltimer = millis();
    clockdisplaypagetimer = _signaltimer;
}

void InternetRadio::setupMQTT()
{
    if(mqttsetup)
        return;

    if(!MQTTConnector.SetupSensor("Station", "sensor", "Internetradio", "", "", "mdi:radio"))
    {
        WebSerialLogger.println("Could not setup Internetradio mqtt!");
        return;
    }


    MQTTConnector.SetupSensor("BytesPlayed", "sensor", "Internetradio", "data_size", "B", "mdi:radio");

    WebSerialLogger.println("Internetradio Sensor mqtt setup done!");

    mqttsetup = true;

    GetStationList();
}

void InternetRadio::SwitchPreset(uint8_t num)
{
    WebSerialLogger.println("InternetRadio::SwitchPreset to " + String(num));
    _current_station_preset = num;
    StartStream(stationlist[_current_station_preset]);
    UpdateMQTT();
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
    LoadPresets();
    
    _current_station_preset = preset;

    StartStream(stationlist[_current_station_preset]);

    UpdateMQTT();
}

void InternetRadio::Stop()
{
    WebSerialLogger.println("InternetRadio stop ...");
    _http.end();
    MP3Player.End();

    SavePresets();
}

void InternetRadio::DisplayInfo()
{
    WebSerialLogger.println("Current stream: " + String(stationlist[_current_station_preset].name));
    WebSerialLogger.println("Played " + String(bytes_served) + " bytes");
}

String InternetRadio::GetFreqDisplayText()
{
    WiFiClient *_client = _http.getStreamPtr();
    if(_client != nullptr && _client->connected())
    {
        Station st = stationlist[_current_station_preset];
        return String(st.name);
    }

    return "Not connected";
}

String InternetRadio::GetClockDisplayText()
{
    if(clockdisplaypage == 0)
    {
        WiFiClient *_client = _http.getStreamPtr();
        if(_client != nullptr && _client->connected())
        {
            Station st = stationlist[_current_station_preset];
            return String(st.name);
        }
        return "Not connected";
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

void InternetRadio::UpdateMQTT()
{
    

    JsonDocument payload;

    WiFiClient *_client = _http.getStreamPtr();
    if(_client != nullptr && _client->connected())
    {
        Station st = stationlist[_current_station_preset];
        payload["Station"] = String(st.name);
        payload["BytesPlayed"] = String(bytes_served);
    }
    else
    {
        payload["Station"] = "Not connected";
        payload["BytesPlayed"] = String(0);
    }
    //WebSerialLogger.println("Sending internetradio mqtt: " + String(st.name) + "/"+ String(bytes_served));

    MQTTConnector.PublishMessage(payload, "Internetradio");
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

        if(mqttsetup)
            UpdateMQTT();
        else if(MQTTConnector.isActive())
            setupMQTT();
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

void InternetRadio::GetStationList()
{
    WebSerialLogger.println("Download internetradio stations!");

    Stations->clear();

    HTTPClient http;
    http.begin("https://de1.api.radio-browser.info/json/stations/search?limit=10&countrycode=DE&hidebroken=true&order=clickcount&reverse=true");
    int httpCode = http.GET();
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      WebSerialLogger.println("[HTTP] GET... code: " +String(httpCode));

      // file found at server
      if (httpCode == HTTP_CODE_OK) {

        // get length of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();
        WebSerialLogger.println("[HTTP] content length: " +String(len));
       
        // get tcp stream
        WiFiClient *stream = http.getStreamPtr();

        JsonDocument doc;
        deserializeJson(doc, *stream);
        JsonArray array = doc.as<JsonArray>();

        int arraysize = array.size();
        WebSerialLogger.println("[HTTP] Entries received: " +String(arraysize));

        for(int i = 0; i < arraysize; i++)
        {
            JsonVariant item = array[i];
            Station *s = new Station();
            s->name = item["name"].as<String>().c_str();
            s->url = item["url"].as<String>().c_str();
            
            Stations->push_back(s);
        }

      }
    } else {
        WebSerialLogger.println("[HTTP] GET... failed, error: " + http.errorToString(httpCode));
      
    }

    http.end();
}

void InternetRadio::LoadPresets()
{
    WebSerialLogger.println("Loading internetradio presets");
   
    Preferences _prefs;
    String prefname = "internetradio";
    if(!_prefs.begin(prefname.c_str(), false)) 
    {
        WebSerialLogger.println("unable to open preferences");
    }
    else
    {
        WebSerialLogger.println("Loading presets from SPIFFS");

        for(int i=0;i<8;i++)
        {
            String name = _prefs.getString(String("STATIONNAME_" + String(i)).c_str(), "");
            String url = _prefs.getString(String("STATIONURL_" + String(i)).c_str(), "");

            if(name != "")
            {
                stationlist[i].name = name.c_str();
                stationlist[i].url = url.c_str();
            }
        }

        _prefs.end();

    }
}

void InternetRadio::SavePresets()
{
    WebSerialLogger.println("Saving internetradio");
    
    Preferences _prefs;
    String prefname = "internetradio";
    WebSerialLogger.println(prefname);
    if(!_prefs.begin(prefname.c_str(), false))
    {
        WebSerialLogger.println("Could not open preset file " + prefname);
        return;
    }

    for(int i=0;i<8;i++)
    {
        _prefs.putString(String("STATIONNAME_" + String(i)).c_str(), stationlist[i].name);
        _prefs.putString(String("STATIONURL_" + String(i)).c_str(), stationlist[i].url);
    }


    _prefs.end();
}