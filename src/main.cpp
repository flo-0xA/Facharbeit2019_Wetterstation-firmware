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
#include <Adafruit_VEML6075.h>
#include <LaCrosse_TX23.h>

/* Konfiguartion für Deepsleep */
#define uS_TO_S_FACTOR 1000000

RTC_DATA_ATTR int bootCount = 0;

bool executed = false;

float temperature = 0;
float humidity = 0;
float pressure = 0;

float uvA;
float uvB;
float uvIndex;

float windSpeed;
int windDirection;

float batteryVoltage;

Adafruit_BME280 temperatureSensor;
Adafruit_VEML6075 uvSensor = Adafruit_VEML6075();
LaCrosse_TX23 windSensor = LaCrosse_TX23(4);

void updateRainfall()
{
    if (!executed)
    {
        //mqtt::publish(TOPIC_RAIN, String(0.272727273).c_str());
        
        Serial.print("Niederschlag gefallen: ");
        Serial.println(0.272727273);
    }
    executed = true;
}

void setup()
{
    Serial.begin(115200);

    // Verbindung mit dem WLAN-Netzwerk herstellen
    //wlan::connect();
    
    // OTA-Prozedur ausführen
    //ota::init(wlan::getClient());

    //ota::exec();

    // Deepsleep konfigurieren
    bootCount++;
    Serial.println("Anzahl der Boots / Starts: " + String(bootCount));

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_2,1);  // Auf Regensensor reagieren
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);  // Auf RTC reagieren

    attachInterrupt(2, updateRainfall, HIGH); // Auf Niederschlag reagieren

    // Verbindung zum MQTT-Broker herstellen
    //mqtt::connect();

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
        
        //mqtt::publish(TOPIC_TEMPERATURE, String(temperature).c_str());
        //mqtt::publish(TOPIC_HUMIDITY, String(humidity).c_str());
        //mqtt::publish(TOPIC_PRESSURE, String(pressure).c_str());

        Serial.print("Temperatur: ");
        Serial.println(temperature);
        Serial.print("Luftfeuchte: ");
        Serial.println(humidity);
        Serial.print("Luftdruck: ");
        Serial.println(pressure);

        if (uvSensor.begin())
        {
            uvA = uvSensor.readUVA();
            uvB = uvSensor.readUVB();

            uvIndex = uvSensor.readUVI();
        } else {
            Serial.println("Kein Kommunikation mit VEML6075 möglich! Verbindung prüfen.");
        }

        //mqtt::publish(TOPIC_UVA, String(uvA).c_str());
        //mqtt::publish(TOPIC_UVB, String(uvB).c_str());     

        Serial.print("UV-A: ");
        Serial.println(uvA);
        Serial.print("UV-B: ");
        Serial.println(uvB);
        Serial.print("UV-Index: ");
        Serial.println(uvIndex);

        if (windSensor.read(windSpeed, windDirection))
        {
            //mqtt::publish(TOPIC_WIND_SPEED, String(windSpeed).c_str());
            //mqtt::publish(TOPIC_WIND_DIRECTION, String(windDirection).c_str());

            Serial.print("Windgeschwindigkeit: ");
            Serial.println(windSpeed);
            Serial.print("Windrichtung: ");
            Serial.println(windDirection);
        }

        // Helligkeit messen
        if (analogRead(4) < 255)
        {
            
        }     
    }
    else
    {
        updateRainfall();
    }
    
    delay(1000);
    
    // Deepsleep ausführen
    Serial.println("ESP wird in Deepsleep versetzt...");
    Serial.flush();

    //mqtt::disconnect();
    //wlan::disconnect();

    esp_deep_sleep_start();
    delay(50);
}

void loop() {
    // Jeglicher Quelltext hier, wird aufgrund des Deepsleeps nicht ausgeführt!
}