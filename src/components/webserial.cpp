#include "webserial.hpp"

String inputString;

void recvMsg(uint8_t *data, size_t len)
{
    Serial.println("Received Data...");
    String d = "";
    for(int i=0; i < len; i++){
        d += char(data[i]);
    }

    inputString = inputString + d;

    Serial.println(d);
}

WebSerialLoggerClass::WebSerialLoggerClass()
{
    Serial.println("Starting webserial server!");
    
}

void WebSerialLoggerClass::Begin(AsyncWebServer *_server)
{
    Serial.println("Starting webserial connection!");

    WebSerial.begin(_server);
    WebSerial.msgCallback(recvMsg);

    running = true;
}

char WebSerialLoggerClass::GetInput()
{
    if(inputString == "")
        return ' ';

    char c = inputString[0];
    inputString = inputString.substring(1);
    return c;
}

bool WebSerialLoggerClass::IsRunning()
{
    return running;
}

void WebSerialLoggerClass::print(const char *text)
{
    Serial.print(text);

    if(running)
        WebSerial.print(text);
}

void WebSerialLoggerClass::print(String text)
{
    Serial.print(text);

    if(running)
        WebSerial.print(text);
}

void WebSerialLoggerClass::println(const char *text)
{
    Serial.println(text);

    if(running)
        WebSerial.println(text);
}

void WebSerialLoggerClass::println(String text)
{
    Serial.println(text);

    if(running)
        WebSerial.println(text);

}

void WebSerialLoggerClass::Flush()
{
    if(running)
        WebSerial.flush();

}

WebSerialLoggerClass WebSerialLogger;