/*
 * 
 */ 
#include "GlobalDefinitions.hpp"
#include "WlanConnector.hpp"
#include "OtaConnector.hpp"
#include "MqttConnector.hpp"

#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SparkFun_VEML6075_Arduino_Library.h>
#include <LaCrosse_TX23.h>

/* Konfiguartion für Deepsleep */
#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP 60

RTC_DATA_ATTR int bootCount = 0;

float temperature = 0;
float humidity = 0;
float pressure = 0;

float uvA = 0;
float uvB = 0;

float windSpeed;
int windDirection;

float batteryVoltage;

Adafruit_BME280 temperatureSensor;
VEML6075 uvSensor;
LaCrosse_TX23 windSensor = LaCrosse_TX23(39);

void setup()
{
    Serial.begin(115200);

    // Verbindung mit dem WLAN-Netzwerk herstellen
    wlan::connect();
    
    // OTA-Prozedur ausführen
    ota::init(wlan::getClient());

    ota::exec();

    // Deepsleep konfigurieren
    bootCount++;
    Serial.println("Anzahl der Boots / Starts: " + String(bootCount));

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1);  // Auf Regensensor reagieren
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);  // Auf RTC reagieren


    // Verbindung zu MQTT-Broker herstellen
    mqtt::connect();

    // Sensor Daten verarbeiten
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
        
        mqtt::publish(TOPIC_TEMPERATURE, String(temperature).c_str());
        mqtt::publish(TOPIC_HUMIDITY, String(humidity).c_str());
        mqtt::publish(TOPIC_PRESSURE, String(pressure).c_str());

        if (uvSensor.begin())
        {
            uvA = uvSensor.a();
            uvB = uvSensor.b();
        }
        else
        {
            Serial.println("Keine Kommunikation mit VEML6075 möglich! Verbindung prüfen.");
        }

        mqtt::publish(TOPIC_UVA, String(uvA).c_str());
        mqtt::publish(TOPIC_UVB, String(uvB).c_str());     

        if (windSensor.read(windSpeed, windDirection))
        {
            mqtt::publish(TOPIC_WIND_SPEED, String(windSpeed).c_str());
            mqtt::publish(TOPIC_WIND_DIRECTION, String(windDirection).c_str());
        }

/*         // batteryVoltage = map(analogRead(32), 0.0f, 4095.0f, 0, 100);    // Ausgabe in Prozent
        batteryVoltage = analogRead(32);    // Ausgabe in Volt

        mqtt::publish(TOPIC_BATTERY, String(batteryVoltage).c_str()); */
    }
    else
    {
        mqtt::publish(TOPIC_RAIN, String(0.272727273).c_str());
    }
    
    delay(5000);
    
    // Deepsleep ausführen
    Serial.println("ESP wird in Deepsleep versetzt...");
    Serial.flush();

    mqtt::disconnect();
    wlan::disconnect();

    esp_deep_sleep_start();
}

void loop() {
    // Jeglicher Quelltext hier, wird aufgrund des Deepsleeps nicht ausgeführt!
}