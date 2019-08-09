#include <WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include <SPI.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LaCrosse_TX23.h>

/* Konfiguartion für Deepsleep */
#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP 60

/* Konfiguartion für Sensorik */
#define SEA_LEVEL_PREPRESSURE (1013.25)
#define WIND_SENSOR_DATA_PIN 18

/* Variablen im Deppsleep beibehalten */
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR float rainValue = 0.272727273F;

/* Konfiguration für MQTT - Kommunikation */
const char* SSID = "IoTify";
const char* PSK = "wBox2019";
const char* MQTT_BROKER = "192.168.178.46";

/* MQTT topics */
const char* topic_temperature = "/wetterstation/temperature";
const char* topic_humidity = "/wetterstation/humidity";
const char* topic_pressure = "/wetterstation/pressure";
const char* topic_windSpeed = "/wetterstation/wind_speed";
const char* topic_windDirection = "/wetterstation/wind_direction";
const char* topic_rain = "/wetterstation/rain";

/* Definitionen bzw. Deklaration für den Windsensor  */
const char* directionTable[] = {"N","NNE","NE","ENE","E","ESE","SE","SSE","S","SSW","SW","WSW","W","WNW","NW","NNW"};
float speed;    // Windgeschwindigkeit in m/s
int direction;  // Index der directionTable (Himmelsrichtung)
 
/* Deklaration für den Temperatursensor */
bool status;
float temperature;
float humidity;
float pressure;

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_BME280 tempSensor;
LaCrosse_TX23 windSensor = LaCrosse_TX23(WIND_SENSOR_DATA_PIN);

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
        Serial.println("Reconnecting...");
        if (!client.connect("ESP32_Weatherstation")) {
            Serial.print("...failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying...");
            delay(5000);
        }
    }
}

int wakeup_reason() {
    return esp_sleep_get_wakeup_cause();
}

void callback() {}
 
void setup() {
    Serial.begin(115200);

    ++bootCount;
    Serial.println("Anzahl der Boots / Starts: " + String(bootCount));

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1);  // Auf Regensensor reagieren
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);  // Auf RTC reagieren

    setup_wifi();
    client.setServer(MQTT_BROKER, 1883);

    // Temperatursensor initialisieren bzw. starten
    status = tempSensor.begin();

    // Fehlerbehandlung falls der Sensor nicht funktioniert
    if(!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(tempSensor.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
    }

    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    if (wakeup_reason() == ESP_SLEEP_WAKEUP_TIMER) {
        /* Temperatursensor auslesen */
        temperature = tempSensor.readTemperature();
        humidity = tempSensor.readHumidity();
        pressure = tempSensor.readPressure() / 100.0F;

        client.publish(topic_temperature, String(temperature).c_str());
        client.publish(topic_humidity, String(humidity).c_str());
        client.publish(topic_pressure, String(pressure).c_str());

        /* Windsensor bzw. Anemometer auslesen */
        if(windSensor.read(speed, direction)) {
            client.publish(topic_windSpeed, String(speed, 1).c_str());
            client.publish(topic_windDirection, directionTable[direction]);
        }
    } else {
        rainValue += 0.272727273F;
        client.publish(topic_rain, String(rainValue).c_str());
    }

    delay(5000);

    Serial.println("Going to sleep now...");
    Serial.flush();

    /* Jegliche Verbindungen trennen */
    client.disconnect();
    WiFi.disconnect();

    esp_deep_sleep_start();
}

void loop() {}