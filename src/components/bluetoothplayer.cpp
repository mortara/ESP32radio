#include "bluetoothplayer.hpp"
#include "bluetoothsink.h"
#include "webserial.hpp"

void BlueToothPlayer::handle_stream() {
  if (circBuffer.available()) { 
    
    if (MP3Player.ReadyForData()) { // Does the VS1053 want any more data (yet)?
      int bytesRead = circBuffer.read((char*)MP3Player.Mp3buffer, BUFFSIZE);
      
      // If we didn't read the full 32 bytes, that's a worry
      if (bytesRead != BUFFSIZE) Serial.printf("Only read %d bytes from  circular buffer\n", bytesRead);
      
      MP3Player.PlayData(bytesRead); // Actually send the data to the VS1053
    }
  }
}

BlueToothPlayer::BlueToothPlayer()
{
    WebSerialLogger.println("BlueToothPlayer setup ...");

    //_a2dp_sink = new BluetoothA2DPSink();

    //_a2dp_sink->set_stream_reader(read_data_stream, false);
    //_a2dp_sink->set_avrc_metadata_callback(avrc_metadata_callback);
}

void BlueToothPlayer::Start()
{
    WebSerialLogger.println("BlueToothPlayer start ...");
    MP3Player.Begin();
    
    WebSerialLogger.println("Waiting for bluetooth ...");
    //_a2dp_sink->start(BLUETOOTH_NAME);

    WebSerialLogger.println("prepare buffer ...");
    circBuffer.flush();
    delay(100);
    circBuffer.write((char *)bt_wav_header, 44);

    WebSerialLogger.println("bluetooth ready ...");
}

void BlueToothPlayer::Stop()
{
    WebSerialLogger.println("BlueToothPlayer stop ...");
    MP3Player.End();
}

void BlueToothPlayer::Loop(char ch)
{
    handle_stream();
}