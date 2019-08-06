#include <WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include <SPI.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LaCrosse_TX23.h>

/* Konfiguartion für Sensorik */
#define SEA_LEVEL_PREPRESSURE (1013.25)
#define DATA_PIN 10

/* Konfiguration für MQTT - Kommunikation */
const char* SSID = "IoTify";
const char* PSK = "wBox2019";
const char* MQTT_BROKER = "192.168.178.46";

/* Definitionen bzw. Deklaration für den Windsensor  */
const char* directionTable[] = {"N","NNE","NE","ENE","E","ESE","SE","SSE","S","SSW","SW","WSW","W","WNW","NW","NNW"};
float speed;    // Windgeschwindigkeit in m/s
int direction;  // Index der directionTable (Himmelsrichtung)
 
WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_BME280 tempSensor;
LaCrosse_TX23 windSensor = LaCrosse_TX23(DATA_PIN);

/* Verbindung zum WLAN-Netzwerk aufbauen */
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to network: ");
    Serial.println(SSID);
 
    WiFi.begin(SSID, PSK);
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected successfully to network");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

/* Verbindung bei Abbruch erneut herstellen */
void reconnect() {
    while (!client.connected()) {
        Serial.print("Reconnecting...");
        if (!client.connect("ESP32_Weatherstation")) {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying...");
            delay(5000);
        }
    }
}
 
void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setServer(MQTT_BROKER, 1883);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    client.publish("/home/data", "Hello World");    // Benutzung: publish( MQTT-Topic, Nachricht )
    delay(5000);
}