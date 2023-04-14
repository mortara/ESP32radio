#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "vs1053.hpp"


class InternetRadio
{
    private:
        WiFiClient *_client;
        HTTPClient *_http;
        VS1053Player  *_player ;

        long bytes_served = 0;
        uint8_t current_station_preset = 0;

        uint8_t clockdisplaypage = 0;
        uint16_t clockdisplaypagetimer = 0;

        // https://dispatcher.rndfnk.com/hr/hrinfo/live/mp3/high
        // http://st01.sslstream.dlf.de/dlf/01/128/mp3/stream.mp3
        //String station = "http://st01.sslstream.dlf.de/dlf/01/128/mp3/stream.mp3";

        //structure for station list
        typedef struct {
            String url;  //stream url
            String name; //stations name
        } Station;

        #define STATIONS 8 //number of stations in the list

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
        
        uint8_t *_mp3buff;
    public:
        InternetRadio(VS1053Player *player);
        void Loop(char ch);
        void Stop();
        void Start();
        void StartStream(Station station);
        void SwitchPreset(uint8_t num);

        String GetFreqDisplayText();
        String GetClockDisplayText();
};