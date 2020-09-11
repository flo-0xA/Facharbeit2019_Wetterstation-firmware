/*
 * Firmware für Wetterstation auf Basis des ESP32 (1.0.5)
 * 
 * Unterstützte Sensoren: BME280 und VEML6075, sowie weitere analoge Peripherie
 * Die Datenübertragung erfolgt via WLAN und MQTT. Ein OTA Support ist NICHT vorhaden.
 * 
 * -----------------------------------------------------------------------------------
 * Diese Software steht unter folgender Lizenz:
 * 
 * MIT License
 *
 * Copyright (c) [2020] [Florian Abeln]
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML6075.h>

/* ========  Wichtige Definitionen zur Konfiguration  ======== */

#define TOPIC_TEMPERATURE "/weatherstation/temperature"
#define TOPIC_HUMIDITY "/weatherstation/humidity"
#define TOPIC_PRESSURE "/weatherstation/pressure"
#define TOPIC_UVA "/weatherstation/uv/a"
#define TOPIC_UVB "/weatherstation/uv/b"
#define TOPIC_UV_INDEX "/weatherstation/uv/index"
#define TOPIC_WIND_SPEED "/weatherstation/wind/speed"
#define TOPIC_WIND_DIRECTION "/weatherstation/wind/direction"
#define TOPIC_RAIN "/weatherstation/rain"
#define TOPIC_BATTERY "/weatherstation/battery"
#define TOPIC_ERROR "/weatherstation/error"

#define HOSTNAME "wetter-station"

#define STATION "AP-HFG54JH3"
#define KEY "0854700482640451"

#define MQTT_SERVER "192.168.178.41"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""

/* ======================  ENDE  ====================== */

WiFiClient wifi;
PubSubClient mqtt(wifi);

Adafruit_BME280 temperature_sensor;
Adafruit_VEML6075 uv_sensor = Adafruit_VEML6075();

bool low_power_mode = false;

void IRAM_ATTR ISR()
{
  Serial.println("INFO: triggered rainfall...");
  Serial.printf("Rainfall: %f mm\n", 0.272727273F);

  // Konstanten Wert (0,272727273 mm) bei jedem Impuls / Auslösen
  mqtt.publish(TOPIC_RAIN, String(0.272727273F).c_str());

  Serial.println("INFO: will sleep...");
  delay(1000);
  Serial.flush();

  mqtt.disconnect();
  WiFi.disconnect();

  esp_deep_sleep_start();
}

void setup()
{
  Serial.begin(115200);

  // Stromspar- und Interrupt Konfiguration
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);

  if (low_power_mode)
  {
    esp_sleep_enable_timer_wakeup(10800 * 1000000LL);
    Serial.println("INFO: low-power-mode active! sleep time set to 180 minutes");
  }
  else
  {
    esp_sleep_enable_timer_wakeup(3600 * 1000000LL);
    Serial.println("INFO: sleep time set to 60 minutes");
  }

  attachInterrupt(GPIO_NUM_33, ISR, HIGH);

  // WLAN initialisieren
  WiFi.setHostname(HOSTNAME);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(STATION, KEY);

  if (!WiFi.waitStatusBits(STA_HAS_IP_BIT, 5000))
  {
    Serial.println("ERROR: wifi failed...will restart");
    delay(1000);
    Serial.flush();

    ESP.restart();
  }

  // MQTT initialisieren
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);

  if (sizeof(MQTT_PASSWORD) > 0)
  {
    mqtt.connect(HOSTNAME, MQTT_USER, MQTT_PASSWORD);
  }

  mqtt.connect(HOSTNAME);

  if (!mqtt.connected())
  {
    Serial.println("ERROR: mqtt failed...will restart");
    delay(1000);
    Serial.flush();

    ESP.restart();
  }

  // Sensoren initialieren
  bool temperature_sensor_status = temperature_sensor.begin(0x76);
  bool uv_sensor_status = uv_sensor.begin();

  // Niederschlagssensor erfassen
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0)
  {
    ISR();

    Serial.flush();
    delay(1000);
    esp_deep_sleep_start();
  }

  // Messung von Temperatur, Luftfeuchte und Luftdruck
  if (temperature_sensor_status)
  {
    float temperature = temperature_sensor.readTemperature();
    float humidity = temperature_sensor.readHumidity();
    float pressure = temperature_sensor.readPressure();

    Serial.println("INFO: measurement started...");
    Serial.printf("Temperature: %f °C\n", temperature);
    Serial.printf("Humidity: %f \%\n", humidity);
    Serial.printf("Pressure: %f hPa\n", pressure);

    mqtt.publish(TOPIC_TEMPERATURE, String(temperature).c_str());
    mqtt.publish(TOPIC_HUMIDITY, String(humidity).c_str());
    mqtt.publish(TOPIC_PRESSURE, String(pressure).c_str());
  }
  else
  {
    Serial.println("ERROR: BME280 failed");
    mqtt.publish(TOPIC_ERROR, "temperature_measurement failed");
  }

  // Messung der UV-Strahlung
  if (uv_sensor_status)
  {
    float uv_a = abs(uv_sensor.readUVA());
    float uv_b = abs(uv_sensor.readUVB());
    float uv_index = abs(uv_sensor.readUVI());

    Serial.println("INFO: measurement started...");
    Serial.printf("UV-A: %f\n", uv_a);
    Serial.printf("UV-B: %f\n", uv_b);
    Serial.printf("UV index: %f\n", uv_index);

    mqtt.publish(TOPIC_UVA, String(uv_a).c_str());
    mqtt.publish(TOPIC_UVB, String(uv_b).c_str());
    mqtt.publish(TOPIC_UV_INDEX, String(uv_index).c_str());
  }
  else
  {
    Serial.println("ERROR: VEML6075 failed");
    mqtt.publish(TOPIC_ERROR, "uv_measurement failed");
  }

  // Ladestand des Akkus erfassen
  float battery_value = analogRead(10);

  if (battery_value <= 3.0)
  {
    low_power_mode = true;
  }
  else
  {
    low_power_mode = false;
  }

  Serial.printf("Battery value: %f\n", battery_value);
  mqtt.publish(TOPIC_BATTERY, String(battery_value).c_str());

  Serial.println("INFO: will sleep...");
  delay(1000);
  Serial.flush();

  mqtt.disconnect();
  WiFi.disconnect();

  esp_deep_sleep_start();
}

void loop() {}
