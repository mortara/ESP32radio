#include "webserial.hpp"

String inputString;

void recvMsg(const uint8_t *data, size_t len)
{
    Serial0.println("Received Data...");
    String d = "";
    for(int i=0; i < len; i++){
        d += char(data[i]);
    }

    inputString = inputString + d;

    Serial0.println(d);
}

void WebSerialLoggerClass::Start()
{
    Serial0.println("Starting webserial server!");
    
}

void WebSerialLoggerClass::Begin(AsyncWebServer *_server)
{
    Serial0.println("Starting webserial connection!");

    WebSerial.begin(_server);
    WebSerial.onMessage(recvMsg);

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
    Serial0.print(text);

    if(running)
        WebSerial.print(text);
}

void WebSerialLoggerClass::print(String text)
{
    Serial0.print(text);

    if(running)
        WebSerial.print(text);
}

void WebSerialLoggerClass::println(const char *text)
{
    Serial0.println(text);

    if(running)
        WebSerial.println(text);
}

void WebSerialLoggerClass::println(String text)
{
    Serial0.println(text);

    if(running)
        WebSerial.println(text);

}

void WebSerialLoggerClass::Loop()
{
    if(running)
       WebSerial.loop();
}

void WebSerialLoggerClass::Flush()
{
    if(running)
        WebSerial.flush();
}

WebSerialLoggerClass WebSerialLogger;