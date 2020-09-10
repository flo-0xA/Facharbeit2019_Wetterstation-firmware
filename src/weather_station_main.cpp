#include "setup.h"
#include "tx23_custom.h"

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML6075.h>


WiFiClient wifi;
PubSubClient mqtt(wifi);

Adafruit_BME280 temperature_sensor;
Adafruit_VEML6075 uv_sensor = Adafruit_VEML6075();
//TX23_Custom wind_sensor;

RTC_DATA_ATTR int boot_count = 0;
bool low_power_mode = false;


void IRAM_ATTR ISR()
{
  Serial.println("INFO: triggered rainfall...");
  Serial.printf("Rainfall: %f mm\n", 0.272727273F);

  // Wert (0,272727273 mm/Tick) übertragen
  mqtt.publish(TOPIC_RAIN, String(0.272727273).c_str());

  Serial.println("INFO: will sleep...");
  delay(1000);
  Serial.flush();

  mqtt.disconnect();
  WiFi.disconnect();

  esp_deep_sleep_start();
}

void setup()
{
  Serial.begin(9600);

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

  boot_count++;
  Serial.printf("INFO: %i. boot\n", boot_count);
  
  // WLAN initialisieren
  //WiFi.setHostname(HOSTNAME);
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
  // bool wind_sensor_status = wind_sensor.begin();

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

  // Messung der Windgeschwindigkeit
  // if (wind_sensor_status)
  // {
  //   Serial.println("INFO: measurement started...");
  //   Serial.printf("Wind speed: %f\n", wind_speed);

  //   mqtt.publish(TOPIC_WIND_SPEED, String(wind_speed, 1).c_str());
  // }
  // else
  // {
  //   Serial.println("ERROR: TX23 failed");
  //   mqtt.publish(TOPIC_ERROR, "wind_measurement failed");
  // }

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