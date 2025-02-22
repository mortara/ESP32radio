#include "wifimanager.hpp"
#include "mqtt.hpp"

void WIFIManagerClass::StartUp()
{
    WiFi.disconnect();

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(true);
    WiFi.setAutoReconnect(true);
    //Connect();

    WiFi.setHostname("ESP32Radio");
}

bool WIFIManagerClass::Connect()
{
    if(WiFi.status() == WL_CONNECTED || connecting)
    {
        //WebSerialLogger.println("Already connected or connecting");
        return true;
    }
    
    connecting = true;
    WiFi.disconnect();
    WiFi.begin(_credentials.SSID.c_str(), _credentials.PASS.c_str());
    WebSerialLogger.println("Connecting to WiFi ..");
    _lastConnectionTry = millis();
    /*int timeout = 10;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
        timeout--;
        if(timeout == 0)
        {
            Serial.println("WIFI timeout ......");
            return false;
        }
    }
    connected = true;
    DisplayInfo();*/
    return true;
}

void WIFIManagerClass::setupMQTT()
{
    if(mqttsetup)
        return;

    WebSerialLogger.println("Setting up Wifi MQTT client");
    
    if(!MQTTConnector.SetupSensor("SSID", "sensor", "WIFI", "", "", ""))
    {
        WebSerialLogger.println("Unable to setup WIFI MQTT client");            
        return;
    }

    MQTTConnector.SetupSensor("BSSID", "sensor", "WIFI", "", "", "");
    MQTTConnector.SetupSensor("WIFI_RSSI", "sensor", "WIFI", "signal_strength", "dB", "mdi:sine-wave");
    MQTTConnector.SetupSensor("Hostname", "sensor", "WIFI", "", "", "");
    MQTTConnector.SetupSensor("IP", "sensor", "WIFI", "", "", "");
    MQTTConnector.SetupSensor("SubnetMask", "sensor", "WIFI", "", "", "");
    MQTTConnector.SetupSensor("Gateway", "sensor", "WIFI", "", "", "");
    MQTTConnector.SetupSensor("DNS", "sensor", "WIFI", "", "", "");

    WebSerialLogger.println("WIfi mqtt setup done!");

    mqttsetup = true;
}

void WIFIManagerClass::Disconnect()
{
    WebSerialLogger.println("disonnecting from WiFi ..");
    WiFi.disconnect();
    
    connecting = false;
    connected = false;
}

void WIFIManagerClass::DisplayInfo(){
     
    WebSerialLogger.print("[*] Network information for ");
    WebSerialLogger.println(_credentials.SSID);

    WebSerialLogger.println("[+] BSSID : " + WiFi.BSSIDstr());
    WebSerialLogger.print("[+] Gateway IP : ");
    WebSerialLogger.println(WiFi.gatewayIP().toString());
    WebSerialLogger.print("[+] DNS IP : ");
    WebSerialLogger.println(WiFi.dnsIP().toString());   
    WebSerialLogger.println((String)"[+] RSSI : " + String(WiFi.RSSI()) + " dB");
    WebSerialLogger.print("[+] ESP32 IP : ");
    WebSerialLogger.println(WiFi.localIP().toString());
    WebSerialLogger.print("[+] Subnet Mask : ");
    WebSerialLogger.println(WiFi.subnetMask().toString());
    
}

bool WIFIManagerClass::IsConnected()
{
    return connected;
}

unsigned long WIFIManagerClass::LastConnectionTry()
{
    return _lastConnectionTry;
}

void WIFIManagerClass::Loop()
{
    unsigned long currentMillis = millis();

    connected = WiFi.isConnected();

    if(connecting && connected)
    {
        WebSerialLogger.println("WiFi connected!");
        connecting = false;
    }

    if(currentMillis - _lastMqttupdate > 30000 && connected && MQTTConnector.isActive())
    {
        if(!mqttsetup)
        {
            setupMQTT();
        }
        else
        {
            JsonDocument payload;
            payload["SSID"] = _credentials.SSID;
            payload["BSSID"] = WiFi.BSSIDstr();
            payload["WIFI_RSSI"] = String(WiFi.RSSI());
            payload["Hostname"] = String(WiFi.getHostname());
            payload["IP"] = WiFi.localIP().toString();
            payload["SubnetMask"] = WiFi.subnetMask().toString();
            payload["Gateway"] = WiFi.gatewayIP().toString();
            payload["DNS"] = WiFi.dnsIP().toString();;

            MQTTConnector.PublishMessage(payload, "WIFI");
            _lastMqttupdate = currentMillis;
        }
    }

    // if WiFi is down, try reconnecting
    if (!connecting && !connected && (currentMillis - _lastConnectionTry >= interval)) 
    {
        WebSerialLogger.println("Reconnecting to WiFi...");
        WiFi.reconnect();
        connecting = true;
        _lastConnectionTry = currentMillis;
    }
}

WIFIManagerClass WIFIManager;