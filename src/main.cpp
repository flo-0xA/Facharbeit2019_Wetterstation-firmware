/*
 * 
 */ 
#include "WlanConnector.hpp"
#include "OtaConnector.hpp"

// #include <Wire.h>
// #include <SPI.h>

// #include <Adafruit_Sensor.h>
// #include <Adafruit_BME280.h>
// #include <LaCrosse_TX23.h>

void setup()
{
    Serial.begin(115200);

    // Verbindung mit dem WLAN-Netzwerk herstellen
    wlan::connect();
    
    // 1.5 OTA-Prozedur ausführen
    ota::init(wlan::getClient());

    ota::exec();

    // 2. Deepsleep konfigurieren


    // 3. Verbindung zu MQTT-Broker herstellen


    // 4. Sensor Daten via MQTT übermitteln


    // 5. Deepsleep ausführen

}

void loop() {
    // Jeglicher Quelltext hier, wird aufgrund des Deepsleeps nicht ausgeführt!
}