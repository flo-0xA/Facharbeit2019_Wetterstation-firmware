#include "WiFi.h"
#include "PubSubClient.h"

/*
** Anzahl der Verbindungsversuche, die durchgeführt werden sollen, bis der Vorganga abgebrochen wird
*/
#define CONNECTION_ATTEMPTS 10

#define WIFI_SSID "AP-test"
#define WIFI_KEY "2124234431346"

#define MQTT_BROKER "hostname"
#define MQTT_PORT 1884

#define UART_BAUDRATE 11520

extern const int us_to_s_factor = 1000000;
extern const int time_to_sleep_daytime = 60;
extern const int time_to_sleep_nighttime = 1800;
extern const int time_to_sleep_lowpower = 3600;

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);


/*
** ESP-Deepsleep mit deffiniertem Interruptpin initialisieren
**
** Der ESP wacht auf, wenn Pin 33 auf LOW gezogen wird.
*/
void deepsleep_init()
{
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1);
}

/*
** Serielle Schnittstelle mit der definierten Baudrate öffnen
*/
bool interface_init()
{
    Serial.begin(UART_BAUDRATE);
    Serial.println("INFO: Serielle Schnittstelle gestartet...");
}

/*
** Verbindung mit dem WLAN-Accesspoint herstellen
*/
bool wifi_init()
{
    WiFi.begin(WIFI_SSID, WIFI_KEY);

    Serial.printf("INFO: Verbinden mit %s AP ...", WIFI_SSID);

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

/*
** Verbindung mit dem MQTT Broker herstellen
*/
bool mqtt_init()
{
    Serial.printf("INFO: Verbinden mit %s Broker ...", MQTT_BROKER);
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