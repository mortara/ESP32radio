#include <Arduino.h>
#include "BluetoothA2DPSink.h"
#include <cbuf.h>
#include "vs1053.hpp"
#include "webserial.hpp"

#ifndef BLUETOOTPLAYER_h
#define BLUETOOTPLAYER_h

#define BLUETOOTH_NAME "Siemens RS-555"

class BlueToothPlayer
{
    private:
        BluetoothA2DPSink *_a2dp_sink;
        
        void handle_stream();
       
        #define BUFFSIZE 32 
        uint8_t *_mp3buff;
        bool _active = false;

    public:
        BlueToothPlayer();
        void Loop(char ch);
        void Stop();
        void Start();
};

#endif