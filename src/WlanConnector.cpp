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
        Serial.print("Verbinden mit ");
        Serial.println(WLAN_SSID);

        WiFi.begin(WLAN_SSID, WLAN_KEY);

        while (WiFi.status() == WL_CONNECT_FAILED)
        {
            delay(50);
            Serial.print(".");
        }

        Serial.print("Erfolgreich mit ");
        Serial.print(WLAN_SSID);
        Serial.println(" verbunden");

        Serial.print("Aktuelle IP(v4)-Adresse: ");
        Serial.println(WiFi.localIP());   
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