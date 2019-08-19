/*
 * 
 */ 
#include "WlanConnector.hpp"
#include "OtaConnector.hpp"
#include "MqttConnector.hpp"

// #include <Wire.h>

// #include <Adafruit_Sensor.h>
// #include <Adafruit_BME280.h>
// #include <LaCrosse_TX23.h>

/* Konfiguartion für Deepsleep */
#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP 60

RTC_DATA_ATTR int bootCount = 0;

void setup()
{
    Serial.begin(115200);

    // Verbindung mit dem WLAN-Netzwerk herstellen
    wlan::connect();
    
    // 1.5 OTA-Prozedur ausführen
    ota::init(wlan::getClient());

    ota::exec();

    // 2. Deepsleep konfigurieren
    bootCount++;
    Serial.println("Anzahl der Boots / Starts: " + String(bootCount));

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1);  // Auf Regensensor reagieren
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);  // Auf RTC reagieren


    // 3. Verbindung zu MQTT-Broker herstellen
    mqtt::connect();

    // 4. Sensor Daten via MQTT übermitteln
    

    // 5. Deepsleep ausführen
    Serial.println("ESP wird in Deepsleep versetzt...");
    Serial.flush();

    // mqttClient.disconnect();
    wlan::disconnect();

    esp_deep_sleep_start();

}

void loop() {
    // Jeglicher Quelltext hier, wird aufgrund des Deepsleeps nicht ausgeführt!
}