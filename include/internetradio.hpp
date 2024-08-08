#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "vs1053.hpp"
#include "dacindicator.hpp"

class InternetRadio
{
    private:
        
        HTTPClient _http;
       
        bool _active = false;
        unsigned long bytes_served = 0;
        uint8_t _current_station_preset = 0;

        uint8_t clockdisplaypage = 0;
        unsigned long clockdisplaypagetimer = 0;
        unsigned long _connectiontimer = 0;
        unsigned long _signaltimer = 0;

        bool mqttsetup = false;
        void setupMQTT();

        // https://dispatcher.rndfnk.com/hr/hrinfo/live/mp3/high
        // http://st01.sslstream.dlf.de/dlf/01/128/mp3/stream.mp3
        //String station = "http://st01.sslstream.dlf.de/dlf/01/128/mp3/stream.mp3";

        //structure for station list
        typedef struct {
            const char *url;  //stream url
            const char *name; //stations name
        } Station;

        #define STATIONS 8 //number of stations in the list
        #define COUNTRIES 10
        #define CATEGORIES 9
        //station list can easily be modified to support other stations
        Station stationlist[STATIONS] = {
        {"http://dispatcher.rndfnk.com/hr/hrinfo/live/mp3/high","HR-Info"},
        {"http://st01.sslstream.dlf.de/dlf/01/128/mp3/stream.mp3","Deutschlandfunk"},
        {"http://dispatcher.rndfnk.com/br/br3/live/mp3/low","Bayern3"},
        {"http://dispatcher.rndfnk.com/hr/hr3/live/mp3/48/stream.mp3","Hessen3"},
        {"http://stream.1a-webradio.de/saw-rock/","Radio 1A Rock"},
        {"http://dispatcher.rndfnk.com/br/brklassik/live/mp3/low","Bayern Klassik"},
        {"http://dispatcher.rndfnk.com/rbb/rbb888/live/mp3/mid","RBB"},
        {"http://rnrw.cast.addradio.de/rnrw-0182/deinrock/low/stream.mp3","NRW Rockradio"}};
        
        std::vector<Station *>* Stations;
        uint8_t seekindex = 0;
        uint8_t seekpage = 0;
        uint8_t seek_country = 0;
        uint8_t seek_category = 0;

        bool updatelistrequested = false;
        unsigned long updatelistmillis = 0;
        bool stationswitchrequested = false;
        unsigned long stationswitchmillis = 0;
        bool seekmode = false;
        const char *countries[COUNTRIES] = {"ALL", "DE", "US", "FR", "GB", "IT", "CA", "JP", "SE", "IE"};
        const char *categories[CATEGORIES] = {"ALL", "pop", "music", "news", "rock", "classical", "talk", "hits", "radio"};
    public:
        InternetRadio();
        void Loop(char ch);
        void Stop();
        uint8_t Start();
        void StartStream(Station &station);
        void SwitchPreset(uint8_t num);
        void UpdateMQTT();
        void DisplayInfo();
        uint8_t GetStationList();
        String GetFreqDisplayText();
        String GetClockDisplayText();
        void LoadPresets();
        void SavePresets();
};