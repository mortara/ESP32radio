#include "bluetoothplayer.hpp"
#include "bluetoothsink.h"


void BlueToothPlayer::handle_stream() {
  if (circBuffer.available()) { 
    
    if (_player->ReadyForData()) { // Does the VS1053 want any more data (yet)?
      int bytesRead = circBuffer.read((char *)_mp3buff, BUFFSIZE);
      
      // If we didn't read the full 32 bytes, that's a worry
      if (bytesRead != BUFFSIZE) Serial.printf("Only read %d bytes from  circular buffer\n", bytesRead);
      
      _player->PlayData(_mp3buff, bytesRead); // Actually send the data to the VS1053
    }
  }
}

BlueToothPlayer::BlueToothPlayer(VS1053Player *player)
{
    Serial.println("BlueToothPlayer setup ...");

    _player = player;
    _mp3buff = new uint8_t[BUFFSIZE]();
    _a2dp_sink = new BluetoothA2DPSink();

    _a2dp_sink->set_stream_reader(read_data_stream, false);
    _a2dp_sink->set_avrc_metadata_callback(avrc_metadata_callback);
}

void BlueToothPlayer::Start()
{
    Serial.println("BlueToothPlayer start ...");
    _player->Begin();
    
    Serial.println("Waiting for bluetooth ...");
    _a2dp_sink->start(BLUETOOTH_NAME);

    Serial.println("prepare buffer ...");
    circBuffer.flush();
    delay(100);
    circBuffer.write((char *)bt_wav_header, 44);

    Serial.println("bluetooth ready ...");
}

void BlueToothPlayer::Stop()
{
    Serial.println("BlueToothPlayer stop ...");
    _player->End();
    
}

void BlueToothPlayer::Loop(char ch)
{
    handle_stream();
}