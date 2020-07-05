#include "WiFi.h"
#include "PubSubClient.h"

#define US_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP 60

#define CONNECTION_ATTEMPTS 10

#define WIFI_SSID "AP-test"
#define WIFI_KEY "2124234431346"

#define MQTT_BROKER "hostname"
#define MQTT_PWD ""
#define MQTT_PORT 1884

#define UART_BAUDRATE 11520

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);


void deepsleep_init()
{
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1);
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * US_TO_S_FACTOR);
}

bool interface_init()
{
    Serial.begin(UART_BAUDRATE);
    Serial.println("Serielle Schnittstelle gestartet...");
}

bool wifi_init()
{
    WiFi.begin(WIFI_SSID, WIFI_KEY);

    Serial.printf("Verbinden mit %d AP ...", WIFI_SSID);

    for (size_t i = 0; i < CONNECTION_ATTEMPTS; i++)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println(" verbunden");
            return true;
        }
        Serial.print(".");
    }
    return false;
}

bool mqtt_init()
{
    Serial.printf("Verbinden mit %d Broker ...", MQTT_BROKER);
    mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);

    for (size_t i = 0; i < CONNECTION_ATTEMPTS; i++)
    {
        if (mqtt_client.connected())
        {
            Serial.println(" verbunden.");
            return true;
        }
        delay(50);
    }
    return false;    
}