#include "internetradio.hpp"

#define MP3buffersize 32

InternetRadio::InternetRadio(VS1053Player *player, DACIndicator *freq, DACIndicator *signal)
{
    Serial.println("InternetRadio setup ...");
    _mp3buff = new uint8_t[MP3buffersize];
    _player = player;

    _pwmindicator_freq = freq;
    _pwmindicator_signal = signal;
    _pwmindicator_signal->SetRange(0, 120);

    _http = new HTTPClient();
    _http->setConnectTimeout(5000);
    _http->setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    _signaltimer = millis();
}

void InternetRadio::SwitchPreset(uint8_t num)
{
    Serial.print("InternetRadio::SwitchPreset to " + String(num));
    current_station_preset = num;
    StartStream(stationlist[current_station_preset]);
}

void InternetRadio::StartStream(Station station)
{
    Serial.print("connecting to ");
    Serial.println(station.url);

    _client = NULL;
    
    // Your Domain name with URL path or IP address with path
    _http->begin(station.url);
    
    // Send HTTP GET request
    int httpResponseCode = _http->GET();
    
    Serial.print("return code: ");
    Serial.println(String(httpResponseCode));

    if (httpResponseCode>0) {
        if(httpResponseCode == HTTP_CODE_OK) {
                // get tcp stream
                _client = _http->getStreamPtr();

                if(_client->connected())
                    Serial.println("Stream started!");
        }
    }
}

void InternetRadio::Start()
{
    Serial.println("InternetRadio start ...");

    _player->Begin();
    StartStream(stationlist[current_station_preset]);
}

void InternetRadio::Stop()
{
    Serial.println("InternetRadio stop ...");
    _http->end();
    _player->End();
}

String InternetRadio::GetFreqDisplayText()
{
    Station st = stationlist[current_station_preset];
    return String(st.name);
}

String InternetRadio::GetClockDisplayText()
{
    if(clockdisplaypage == 0)
    {
        Station st = stationlist[current_station_preset];
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

void InternetRadio::Loop(char ch)
{
    uint16_t now = millis();
    if((now - clockdisplaypagetimer) > 5000)
    {
        clockdisplaypage++;
        if(clockdisplaypage == 4)
            clockdisplaypage = 0;
        
        clockdisplaypagetimer = now;

        
    }

    if(now - _signaltimer > 500)
    {
        _pwmindicator_signal->SetValue((uint16_t)abs(WiFi.RSSI()));
        _signaltimer = now;
    }

    if(_client != NULL && _client->connected())
    {
        if (_player->ReadyForData()) {
                if (_client->available() >= MP3buffersize) {
                //Serial.println("playing data");
                // The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
                uint8_t bytesread = _client->read(_mp3buff, MP3buffersize);
                bytes_served += bytesread;
                if(bytesread > 0)
                {
                    _player->PlayData(_mp3buff, bytesread);
                    if(bytesread != MP3buffersize)
                        Serial.println("Data buffer != " + String(MP3buffersize) + " bytes");
                }
                else
                    Serial.println("no data in buffer");

            }
        }  

    } 
    else if(WiFi.isConnected())
    {
        delay(100);
        Serial.println("Try to connect ...");
        StartStream(stationlist[current_station_preset]);
    }
    
    if (ch == 'i') 
    {
        Serial.println("Played " + String(bytes_served) + " bytes");
    }
    
}