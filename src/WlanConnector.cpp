/*
 * 
 */
#include "WlanConnector.hpp"

#include "GlobalDefinitions.hpp"

#include <WiFi.h>

namespace wlan
{
    WiFiClient client;

    /*
     * 
     */
    void connect()
    {
        Serial.print("Verbinden mit: " + String(WLAN_SSID));

        WiFi.begin(WLAN_SSID, WLAN_KEY);

        while (WiFi.status() == WL_CONNECT_FAILED)
        {
            delay(50);
            Serial.print(".");
        }

        Serial.print("Erfolgreich mit " + String(WLAN_SSID) + " verbunden");

        Serial.print("Aktuelle IP(v4)-Adresse: " + String(WiFi.localIP()));
    }

    /*
     * 
     */
    void disconnect()
    {
        WiFi.disconnect();
    }

    /*
     *
     */
    bool isConnected()
    {
        return client.connected();
    }

    WiFiClient& getClient()
    {
        return client;
    }
}