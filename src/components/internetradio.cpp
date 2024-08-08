#include "internetradio.hpp"
#include "mqtt.hpp"
#include "tunerbuttons.hpp"
#include <iterator>

InternetRadio::InternetRadio()
{
    WebSerialLogger.println("InternetRadio setup ...");
    
     _http.setConnectTimeout(5000);
    _http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    Stations = new std::vector<Station *>();

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

}

void InternetRadio::SwitchPreset(uint8_t num)
{
    

    WebSerialLogger.println("InternetRadio::SwitchPreset to " + String(num));
    _current_station_preset = num;

    if(seekmode)
        return;

    StartStream(stationlist[_current_station_preset]);
    UpdateMQTT();
}

void InternetRadio::StartStream(Station &station)
{
    _connectiontimer = millis();
    WebSerialLogger.println("connecting to " + String(station.url));
   
    _http.end();
    delay(100);
    bytes_served = 0;
    // Your Domain name with URL path or IP address with path
    _http.begin(station.url);
    
    // Send HTTP GET request
    int httpResponseCode = _http.GET();
    
    WebSerialLogger.print("return code: " + String(httpResponseCode));
 
    if(httpResponseCode == HTTP_CODE_OK) {
       
        if(_http.getStreamPtr() != nullptr)
            WebSerialLogger.println("Stream started!");
        else
        {
            WebSerialLogger.println("Stream not started!");
        }
    }
    
}

uint8_t InternetRadio::Start()
{
    WebSerialLogger.println("InternetRadio start ...");
    SignalIndicator.SetRange(0, 120);
    MP3Player.Begin();
    LoadPresets();
    
    StartStream(stationlist[_current_station_preset]);

    UpdateMQTT();

    return _current_station_preset;
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
        if(seekmode)
        {
            return String(Stations->at(seekindex)->name);
        }

        Station st = stationlist[_current_station_preset];
        return String(st.name);
    }

    return "Not connected";
}

String InternetRadio::GetClockDisplayText()
{
    if(seekmode)
    {
        if(updatelistrequested)
        {
            String result =  String(countries[seek_country]);
            result += "-";
            result += String(categories[seek_category]);

            return result;
        }

        return String(Stations->at(seekindex)->name);
    }

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

void InternetRadio::Loop(char ch)
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
    if(_client != nullptr)
    {
        if (MP3Player.ReadyForData()) 
        {
            int available = _client->available();
            if(available > MP3buffersize)
                available = MP3buffersize;

            if (available >= MinSizeToPlay) {
                //Serial.println("playing data");
                // The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
                uint8_t bytesread = _client->read(MP3Player.Mp3buffer, available);
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
    else if(WiFi.status() == WL_CONNECTED && now - _connectiontimer > 10000)
    {
        WebSerialLogger.println("Try to connect ...");
        StartStream(stationlist[_current_station_preset]);
    }

    if(seekmode)
    {
        if(updatelistrequested && now - updatelistmillis > 3000)
        {
            seekpage = 0;
            GetStationList();
            seekindex = 0;
            updatelistrequested = false;
        } else if(stationswitchrequested && now - stationswitchmillis > 3000)
        {
            stationswitchrequested = false;
            Station *s = Stations->at(seekindex);
            StartStream(*s);

            if(TunerButtons.SavePresetButtonPressed)
            {
                Station ps;
                
                String *name = new String(s->name);
                String *url = new String(s->url);

                ps.name = name->c_str();
                ps.url = url->c_str();

                stationlist[_current_station_preset] = ps;
            }
        }
    }

    switch(ch)
    {
        case 'o': // small step up
            
            WebSerialLogger.println("Next category");
            seek_category++;
            
            if(seek_category >= CATEGORIES)
                seek_category = 0;
            updatelistrequested = true;
            updatelistmillis = millis();
            break;
        case 'i': // small step down
            seek_category--;
            if(seek_category < 0)
                seek_category = CATEGORIES-1;
            WebSerialLogger.println("previous category");
            updatelistrequested = true;
            updatelistmillis = millis();
            break;
        case 'I': // step up
            WebSerialLogger.println("Next country");
            seek_country++;
            
            if(seek_country >= COUNTRIES)
                seek_country = 0;
            updatelistrequested = true;
            updatelistmillis = millis();
            break;
        case 'O': // step down
            seek_country--;
            if(seek_country < 0)
                seek_country = COUNTRIES-1;
            WebSerialLogger.println("previous country");
            updatelistrequested = true;
            updatelistmillis = millis();
            break;
        case 'u': // start seek up
            WebSerialLogger.println("Next station ...");
            stationswitchrequested = true;
            stationswitchmillis = millis();
            seekindex++;
            if(seekindex >= Stations->size());
            {               
                seekpage++;
                uint8_t nstations = GetStationList();
                if(nstations == 0)
                {
                    seekpage--;
                    GetStationList();
                }
                seekindex = 0;
            }
            break;
        case 'z': // start seek down
            WebSerialLogger.println("previous station ...");
            stationswitchrequested = true;
            stationswitchmillis = millis();
            seekindex--;
            if(seekindex < 0)
            {
                if(seekpage>0)
                    seekpage--;

                GetStationList();

                seekindex = Stations->size()-1;
            }
            break;
        case 't': // stop/start seek
            WebSerialLogger.println("");
            if(seekmode)
            {
                updatelistrequested = false;
                stationswitchrequested = false;
                SavePresets();
            }
            else
            {
                updatelistrequested = true;
                updatelistmillis = millis();
            }

            seekmode = !seekmode;
            delay(100);
            break;       
    }

    
}

uint8_t InternetRadio::GetStationList()
{
    WebSerialLogger.println("Download internetradio stations!");

    if(Stations->size() > 0)
    {
        for(int i = 0; i<Stations->size(); i++)
        {
            Station *s = Stations->at(i);
            delete s;
        }
           
    }

    Stations->clear();

    int offset = seekpage * 10;
    String country = String(countries[seek_country]);
    String tags = String(categories[seek_category]);

    String url = "https://de1.api.radio-browser.info/json/stations/search?limit=10&offset="; 
    url += String(offset);

    if(seek_country > 0)
    {
        url += "&countrycode=";
        url += country;
    }

    if(seek_category > 0)
    {
        url += "&tagList=";
        url += tags;
    }
    url += "&hidebroken=true&order=clickcount&reverse=true";

    HTTPClient http;
    http.begin(url);

    WebSerialLogger.println("[HTTP] URL... : " + url);

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

            String *name = new String(item["name"].as<String>());
            String *rurl = new String(item["url"].as<String>());

            if(*name != "" && rurl->startsWith("http"))
            {
                Station *s = new Station();
                s->name = name->c_str();
                s->url =  rurl->c_str();
                
                Stations->push_back(s);

                WebSerialLogger.println(*name + ":" + *rurl);
            }
            
        }

      }
    } else {
        WebSerialLogger.println("[HTTP] GET... failed, error: " + http.errorToString(httpCode));
      
    }

    http.end();

    return Stations->size();
}

void InternetRadio::LoadPresets()
{
    WebSerialLogger.println("Loading internetradio presets");
    _current_station_preset = 0;
   
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
            String *name = new String(_prefs.getString(String("STATIONNAME_" + String(i)).c_str(), ""));
            String *url = new String(_prefs.getString(String("STATIONURL_" + String(i)).c_str(), ""));

            if(*name != "")
            {
                Station s;

                s.name = name->c_str();
                s.url = url->c_str();
                stationlist[i] = s;
            }
        }

        _current_station_preset = _prefs.getUShort("LASTPRESET", 0);

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

    _prefs.putUShort("LASTPRESET", _current_station_preset);

    _prefs.end();
}