#include "wifimanager.hpp"


WIFIManager::WIFIManager()
{
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    //Connect();
}

bool WIFIManager::Connect()
{
    if(WiFi.status() == WL_CONNECTED)
        return true;

    WiFi.begin(_credentials.SSID.c_str(), _credentials.PASS.c_str());
    Serial.println("Connecting to WiFi ..");
    _lastConnectionTry = millis();
    int timeout = 20;
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
    DisplayInfo();
    return true;
}

void WIFIManager::Disconnect()
{
    connected = false;
    Serial.println("disonnecting from WiFi ..");
    WiFi.disconnect();
}

void WIFIManager::DisplayInfo(){
     
    Serial.print("[*] Network information for ");
    Serial.println(_credentials.SSID);

    Serial.println("[+] BSSID : " + WiFi.BSSIDstr());
    Serial.print("[+] Gateway IP : ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("[+] DNS IP : ");
    Serial.println(WiFi.dnsIP());   
    Serial.println((String)"[+] RSSI : " + String(WiFi.RSSI()) + " dB");
    Serial.print("[+] ESP32 IP : ");
    Serial.println(WiFi.localIP());
    Serial.print("[+] Subnet Mask : ");
    Serial.println(WiFi.subnetMask());
    
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
    if (connected && (WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) 
    {
        Serial.println("Reconnecting to WiFi...");
        WiFi.reconnect();
        previousMillis = currentMillis;
    }
}