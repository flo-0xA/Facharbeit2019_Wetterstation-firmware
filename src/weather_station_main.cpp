#include "initialization.h"

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LaCrosse_TX23.h>


#define SEA_LEVEL_PREPRESSURE 1013.25
#define RAIN_LEVEL 0.272727273

#define TOPIC_TEMPERATURE ""
#define TOPIC_HUMIDITY ""
#define TOPIC_PRESSURE ""
#define TOPIC_WIND_SPEED ""
#define TOPIC_WIND_DIRECTION ""
#define TOPIC_RAIN ""
#define TOPIC_BATTERY ""
#define TOPIC_ERROR ""

Adafruit_BME280 temperature_sensor;
LaCrosse_TX23 wind_sensor = LaCrosse_TX23(18);

RTC_DATA_ATTR bool daytime = true;
RTC_DATA_ATTR bool battery_lowpower = false;

bool error_reporting = true;
 

void setup() {
  interface_init();

  deepsleep_init();

  if (battery_lowpower)
  {
    esp_sleep_enable_timer_wakeup(time_to_sleep_lowpower * us_to_s_factor);

    Serial.printf("INFO: Wenig Energie vorhanden. Deepsleep verängert - %i s\n", time_to_sleep_lowpower);
  }
  else if (daytime)
  {
    esp_sleep_enable_timer_wakeup(time_to_sleep_daytime * us_to_s_factor);

    Serial.printf("INFO: Tagmodus ist aktiv. Deepsleep Intervall beträgt %i s\n", time_to_sleep_daytime);
  }
  else
  {
    esp_sleep_enable_timer_wakeup(time_to_sleep_nighttime * us_to_s_factor);

    Serial.printf("INFO: Nachtmodus ist aktiv. Deepsleep Intervall beträgt %i s\n", time_to_sleep_nighttime);
  }
  

  if (!wifi_init())
  {
    Serial.println("ERROR: Verbindung zum AP fehlgeschlagen! Neustart erfolgt...");
    delay(1000);

    ESP.restart();
  }

  if (mqtt_init())
  {
    Serial.println("ERROR: Verbindung zum MQTT Broker fehlgeschlagen! Neustart erfolgt...");
    delay(1000);

    ESP.restart();
  }
  
  bool temperature_sesnor_status = temperature_sensor.begin();

  if (!temperature_sesnor_status)
  {
    Serial.println("ERROR: Verbindung mit BME280 Sensor fehlgeschlagen!");
  }

  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
  {
    // TODO: Nur ausführen, wenn Akkuspannung über Wert X - andernfalls deepsleep ausführen

    if (temperature_sesnor_status)
    {
      float temperature = temperature_sensor.readTemperature();
      float humidity = temperature_sensor.readTemperature();
      float pressure = temperature_sensor.readPressure();

      Serial.println("INFO: Messung gestartet...");
      Serial.printf("Temperatur: %f °C\n", temperature);
      Serial.printf("Luftfeuchte: %f \n", humidity);
      Serial.printf("Luftdruck: %f hPa\n", pressure);

      mqtt_client.publish(TOPIC_TEMPERATURE, String(temperature).c_str());
      mqtt_client.publish(TOPIC_HUMIDITY, String(humidity).c_str());
      mqtt_client.publish(TOPIC_PRESSURE, String(pressure).c_str());
    }
    else if (error_reporting)
    {
      Serial.println("ERROR: Keine Messung von BME280 Sensor erfolgt.");
      mqtt_client.publish(TOPIC_ERROR, "temperature_measurement failed");
    }
    
    float wind_speed;
    int wind_direction_index;

    bool wind_sensor_status = wind_sensor.read(wind_speed, wind_direction_index);

    if (wind_sensor_status)
    {
      const char* direction_table[] = {"N","NNE","NE","ENE","E","ESE","SE","SSE","S","SSW","SW","WSW","W","WNW","NW","NNW"};

      const char* direction = direction_table[wind_direction_index];

      Serial.println("INFO: Messung gestartet...");
      Serial.printf("Windrichtung: %s\n", direction);
      Serial.printf("Windgeschwindigkeit: %f\n", wind_speed);

      mqtt_client.publish(TOPIC_WIND_DIRECTION, direction);
      mqtt_client.publish(TOPIC_WIND_SPEED, String(wind_speed, 1).c_str());
    }
    else if (error_reporting)
    {
      Serial.println("ERROR: Keine Messung von TX23 Sensor erfolgt.");
      mqtt_client.publish(TOPIC_ERROR, "wind_measurement failed");
    }

    float battery_value = analogRead(10);

    battery_lowpower = battery_value < 1000;

    Serial.println("INFO: Wert erfasst...");
    Serial.printf("Akkuspannung: %f V\n", battery_value);

    mqtt_client.publish(TOPIC_BATTERY, String(battery_value).c_str());

    daytime = analogRead(4) < 255;  
  }
  else
  {
    Serial.println("INFO: Wert erfasst...");
    Serial.printf("Niederschlag: %f mm\n", RAIN_LEVEL);

    mqtt_client.publish(TOPIC_RAIN, String(RAIN_LEVEL).c_str());
  }
  
    delay(2000);

    Serial.println("INFO: Deepsleep beginnt in kürze...");
    Serial.flush();

    WiFi.disconnect();
    mqtt_client.disconnect();

    esp_deep_sleep_start();
}

void loop() {}