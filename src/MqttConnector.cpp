/*
 * 
 */
#include "MqttConnector.hpp"
#include "GlobalDefinitions.hpp"
#include "WlanConnector.hpp"

#include <PubSubClient.h>

namespace mqtt
{
    PubSubClient client(wlan::getClient());

    /*
     * 
     */
    void connect()
    {
        client.setServer(MQTT_HOST,  MQTT_PORT);

        Serial.println("Mit " + String(MQTT_HOST) + "-Broker verbinden");

        if (!client.connect("ESP_Weatherstation"))
        {
            delay(500);
            Serial.print(".");
        }

        Serial.println("Mit Broker: " + String(MQTT_HOST) + " erfolgreich verbunden");
    }

    /*
     * 
     */
    void disconnect()
    {
        client.disconnect();
    }

    /*
     * 
     */
    bool isConnected()
    {
        return client.connected();
    }

    /*
     *
     */
    bool publish(char* topic, const char* message)
    {
        return client.publish(topic, message);
    }
}