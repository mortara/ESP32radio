#include "webserver.hpp"

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void handleRoot(AsyncWebServerRequest *request) {
    
    //Serial.println("Webserver handle request ...");

    int sec = millis() / 1000;
    int min = sec / 60;
    int hr = min / 60;

   String header = "\
                    <html>\
                    <head>\
                        <meta http-equiv='refresh' content='15'/>\
                        <title>ESP8266 AIO Sensor</title>\
                        <style>\
                        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
                        </style>\
                    </head>";

    String body = "\
                    <body>\
                        <h1>Hello from  " + String(WiFi.getHostname()) + "!</h1>\
                        <p>Uptime: " + hr +"h " + String(min % 60) + "m " + String(sec % 60) + "s </p>";

   
    String footer = "</body>\
                    </html>";


    String html = header + body + footer;
    request->send(200, "text/html", html);
  
}

void WebServerClass::Setup()
{
    _webserver = new AsyncWebServer(80);
    _webserver->on("/", handleRoot);

    dnsServer.start(53, "*", WiFi.softAPIP());
    _webserver->onNotFound(notFound);
    _webserver->begin();
}

AsyncWebServer *WebServerClass::GetServer()
{
    return _webserver;
}

WebServerClass WebServer;