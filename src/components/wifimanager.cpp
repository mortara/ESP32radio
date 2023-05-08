#include "wifimanager.hpp"


WIFIManager::WIFIManager()
{
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    //Connect();
}

bool WIFIManager::Connect()
{
    if(WiFi.status() == WL_CONNECTED || connecting)
        return true;

    connecting = true;

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

void WIFIManager::Disconnect()
{
    WebSerialLogger.println("disonnecting from WiFi ..");
    WiFi.disconnect();
}

void WIFIManager::DisplayInfo(){
     
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

bool WIFIManager::IsConnected()
{
    return connected;
}

unsigned long WIFIManager::LastConnectionTry()
{
    return _lastConnectionTry;
}

void WIFIManager::Loop(char ch)
{
    connected = WiFi.isConnected();

    if(connecting && connected)
    {
        connecting = false;
    }

    if (ch == 'w') 
    {
        DisplayInfo();    
    }

    if (ch == 'v') 
    {
        Connect();    
    }

    unsigned long currentMillis = millis();
    // if WiFi is down, try reconnecting
    if (!connecting && !connected && (currentMillis - _lastConnectionTry >=interval)) 
    {
        WebSerialLogger.println("Reconnecting to WiFi...");
        WiFi.reconnect();
        connecting = true;
        _lastConnectionTry = currentMillis;
    }
}