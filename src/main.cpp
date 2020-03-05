#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML6075.h>
#include <LaCrosse_TX23.h>

#include <cstdlib>

/* Konfiguration */

// Deepsleep Konfiguration
RTC_DATA_ATTR int bootCount = 0;
#define uS_TO_S_FACTOR 1000000
#define SLEEPTIME 5

// WLAN Verbindungsdaten
const char* SSID = "ssid";
const char* KEY = "123";

const char* HOSTNAME = "test";

// MQTT Konfiguration
#define BROKER "test"
#define PORT 1234

/* - */

Adafruit_BME280 temperatureSensor;
Adafruit_VEML6075 uvSensor = Adafruit_VEML6075();
LaCrosse_TX23 windSensor = LaCrosse_TX23(39);

WiFiClient wlanClient;
PubSubClient mqttClient;

float temperature;
float humidity;
float pressure;

float windSpeed;
int windDirection;

float uvA, uvB, uvIndex;

void connect()
{
    // WLAN Verbindung aufbauen
    Serial.println("Connect to AP:" + String(SSID) + "..");

    WiFi.begin(SSID, KEY);

    while (WiFi.status() == WL_CONNECT_FAILED)
    {
        delay(50);
        Serial.print(".");
    }

    WiFi.setHostname(HOSTNAME);

    Serial.println("\nConnection established (Network: " + String(SSID) + "). Current IP-Address: " + String(WiFi.localIP()));


    // Verbindung mit MQTT Broker aufbauen
    mqttClient = PubSubClient(wlanClient);

    mqttClient.setServer(BROKER, PORT);

    Serial.print("Connect to Mqtt-Broker..");

    while (!mqttClient.connect(HOSTNAME))
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnection established (Broker: " + String(BROKER) + ")");
}

void updateRainfallData()
{

}

void setup() {
    Serial.begin(115200);

    // Verbindung mit WLAN-AP und MQTT-Broker herstellen
//    connect();
    
    bootCount++;
    Serial.println("Boot count: " + String(bootCount));

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);                   // Auf Regensensor reagieren
    esp_sleep_enable_timer_wakeup(SLEEPTIME * uS_TO_S_FACTOR);      // Auf RTC reagieren

    attachInterrupt(33, updateRainfallData, HIGH);                       // Auf Niederschlag reagieren

    // Wakeup reason: vordefinierter Timer
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
    {
        if (temperatureSensor.begin())
        {
            temperature = temperatureSensor.readTemperature();
            humidity = temperatureSensor.readHumidity();
            pressure = temperatureSensor.readPressure();
        }
        else
        {
            Serial.println("Keine Kommunikation mit BME280 möglich! Verbindung prüfen.");
        }
        
        mqttClient.publish("", String(temperature).c_str());
        mqttClient.publish("", String(humidity).c_str());
        mqttClient.publish("", String(pressure).c_str());

        Serial.print("Temperatur: ");
        Serial.println(temperature);
        Serial.print("Luftfeuchte: ");
        Serial.println(humidity);
        Serial.print("Luftdruck: ");
        Serial.println(pressure);

        if (uvSensor.begin())
        {
            uvA = abs(uvSensor.readUVA());
            uvB = abs(uvSensor.readUVB());

            uvIndex = abs(uvSensor.readUVI());
        } else {
            Serial.println("Kein Kommunikation mit VEML6075 möglich! Verbindung prüfen.");
        }

        mqttClient.publish("", String(uvA).c_str());
        mqttClient.publish("", String(uvB).c_str());
        mqttClient.publish("TOPIC_UVI", String(uvIndex).c_str());    

        Serial.print("UV-A: ");
        Serial.println(uvA);
        Serial.print("UV-B: ");
        Serial.println(uvB);
        Serial.print("UV-Index: ");
        Serial.println(uvIndex);

        if (windSensor.read(windSpeed, windDirection))
        {
            mqttClient.publish("", String(windSpeed).c_str());
            mqttClient.publish("", String(windDirection).c_str());

            Serial.print("Windgeschwindigkeit: ");
            Serial.println(windSpeed);
            Serial.print("Windrichtung: ");
            Serial.println(windDirection);
        }

        // Helligkeit messen
        if (analogRead(34) < 255)
        {
            
        }  

        // Akkuspannung auslesen
        int adc_value = analogRead(35);
        Serial.println("ADC value: " + String(adc_value));
        Serial.println("Voltage: " + String((adc_value * 3.3 ) / 4095));
    }
    // Wakeup reason: Niederschlag gefallen / Sensor ausgelöst
    else
    {
        updateRainfallData();
    }

    Serial.flush();
    delay(100);
    esp_deep_sleep_start();
}

void loop() {
  // put your main code here, to run repeatedly:

}