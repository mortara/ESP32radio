#include "internetradio.hpp"

InternetRadio::InternetRadio(VS1053Player *player)
{
    Serial.println("InternetRadio setup ...");
    _mp3buff = new uint8_t[32]();
    _player = player;

    _http = new HTTPClient();
    _http->setConnectTimeout(3000);
    _http->setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
}

void InternetRadio::SwitchPreset(int num)
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
}

String InternetRadio::GetFreqDisplayText()
{
    Station st = stationlist[current_station_preset];
    return st.name;
}

void InternetRadio::Loop(char ch)
{
    if(_client != NULL && _client->connected())
    {
        if (_client->available() > 0) {
            //Serial.println("Receiving data");
            if (_player->ReadyForData()) {
                //Serial.println("playing data");
                // The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
                uint8_t bytesread = _client->read(_mp3buff, 32);
                bytes_served += bytesread;
                if(bytesread > 0)
                {
                    _player->PlayData(_mp3buff, bytesread);
                    if(bytesread != 32)
                        Serial.println("Data buffer != 32 bytes");
                }
                else
                    Serial.println("no data in buffer");

            }
        }  

    } 
    else if(WiFi.isConnected())
    {
        delay(500);
        StartStream(stationlist[current_station_preset]);
    }
    
    if (ch == 'i') 
    {
        Serial.println("Played " + String(bytes_served) + " bytes");
    }
    
}