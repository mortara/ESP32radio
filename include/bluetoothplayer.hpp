
#ifndef BLUETOOTPLAYER_h
#define BLUETOOTPLAYER_h

#include <Arduino.h>
#include "BluetoothA2DPSink.h"
#include <cbuf.h>
#include "vs1053.hpp"

#define BLUETOOTH_NAME "Siemens RS-555"


class BlueToothPlayer
{
    private:
        BluetoothA2DPSink *_a2dp_sink;
        
        void handle_stream();
       
        VS1053Player  *_player ;

        #define BUFFSIZE 32 
        uint8_t *_mp3buff;
        bool _active = false;

    public:
        BlueToothPlayer(VS1053Player *player);
        void Loop(char ch);
        void Stop();
        void Start();
};

#endif