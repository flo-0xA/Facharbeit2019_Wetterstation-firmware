#include "initialization.h"

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML6075.h>
#include <LaCrosse_TX23.h>


#define SEA_LEVEL_PREPRESSURE 1013.25
#define RAIN_LEVEL 0.272727273

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

Adafruit_BME280 temperature_sensor;
Adafruit_VEML6075 uv_sensor = Adafruit_VEML6075();
LaCrosse_TX23 wind_sensor = LaCrosse_TX23(18);

/*
** Zustand des Tag- bzw. Nachmodus
** Zustand des Low-Power-Modus
**
** Variablen bleiben nach Deepsleep bzw. Reboot erhalten.
*/
RTC_DATA_ATTR bool daytime = true;
RTC_DATA_ATTR bool battery_lowpower = false;

bool error_reporting = true;
 

void setup() {
  interface_init();

  deepsleep_init();

  /*
  ** Tag- Nachmodus bzw. Low-Power-Modus
  **
  ** Je nachdem, welcher Fall (Tag, Nacht oder Akkusatnd niedrig) auftritt, wird das Sleep-Intervall auf deffinierte Werte angepasst.
  */
  if (battery_lowpower)
  {
    esp_sleep_enable_timer_wakeup(time_to_sleep_lowpower * us_to_s_factor);

    Serial.printf("INFO: Wenig Energie vorhanden. Deepsleep verängert - %isek\n", time_to_sleep_lowpower);
  }
  else if (daytime)
  {
    esp_sleep_enable_timer_wakeup(time_to_sleep_daytime * us_to_s_factor);

    Serial.printf("INFO: Tagmodus ist aktiv. Deepsleep Intervall beträgt %isek \n", time_to_sleep_daytime);
  }
  else
  {
    esp_sleep_enable_timer_wakeup(time_to_sleep_nighttime * us_to_s_factor);

    Serial.printf("INFO: Nachtmodus ist aktiv. Deepsleep Intervall beträgt %isek\n", time_to_sleep_nighttime);
  }
  

  if (!wifi_init())
  {
    Serial.println("ERROR: Verbindung zum AP fehlgeschlagen! Neustart erfolgt...");
    delay(1000);

    ESP.restart();
  }

  if (!mqtt_init())
  {
    Serial.println("ERROR: Verbindung zum MQTT Broker fehlgeschlagen! Neustart erfolgt...");
    delay(1000);

    ESP.restart();
  }
  
  /*
  ** Temperatur Sensor (BME280) und UV-Sensor (VEML6075) initialisieren (I2C)
  */
  bool temperature_sensor_status = temperature_sensor.begin();
  bool uv_sensor_status = uv_sensor.begin();

  /*
  ** Grund des Aufwachens aus dem Deepsleep
  **
  ** Beim ersten Fall (die vordeffinierte Zeit ist überschritten), werden alle Sensoren überprüft und deren Messwerte übertragen.
  ** Andernfalls wurde der Niederschlagssensor ausgelöst, sodass eine fixe Konstante als Messwert übertragen wird.
  */
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
  {
    /*
    ** Wird nur ausgeführt, wenn der Sensor funktioniert. Andernfalls wird der Fehler via RS-232 dokumentiert.
    */
    if (temperature_sensor_status)
    {
      float temperature = temperature_sensor.readTemperature();
      float humidity = temperature_sensor.readHumidity();
      float pressure = temperature_sensor.readPressure();

      Serial.println("INFO: Messung gestartet...");
      Serial.printf("Temperatur: %f °C\n", temperature);
      Serial.printf("Luftfeuchte: %f \%\n", humidity);
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

    /*
    ** Wird nur ausgeführt, wenn der Sensor funktioniert. Andernfalls wird der Fehler via RS-232 dokumentiert.
    */
    if (uv_sensor_status)
    {
      float uv_a = abs(uv_sensor.readUVA());
      float uv_b = abs(uv_sensor.readUVB());
      float uv_index = abs(uv_sensor.readUVI());

      Serial.println("INFO: Messung gestartet...");
      Serial.printf("UV-A: %f\n", uv_a);
      Serial.printf("UV-B: %f\n", uv_b);
      Serial.printf("UV Index: %f\n", uv_index);

      mqtt_client.publish(TOPIC_UVA, String(uv_a).c_str());
      mqtt_client.publish(TOPIC_UVB, String(uv_b).c_str());
      mqtt_client.publish(TOPIC_UV_INDEX, String(uv_index).c_str());
    }
    else if (error_reporting)
    {
      Serial.println("ERROR: Keine Messung von VEML6075 Sensor erfolgt.");
      mqtt_client.publish(TOPIC_ERROR, "uv_measurement failed");
    }
    
    float wind_speed;
    int wind_direction_index;

    /*
    ** Wind Sensor (TX23) initialisieren und Messwerte ermitteln
    */
    bool wind_sensor_status = wind_sensor.read(wind_speed, wind_direction_index);

    /*
    ** Wird nur ausgeführt, wenn der Sensor funktioniert. Andernfalls wird der Fehler via RS-232 dokumentiert.
    */
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

    /*
    ** Low-Power-Modus aktivieren wenn der Schwellenwert unterschritten wurde
    */
    battery_lowpower = battery_value < 1000;

    Serial.println("INFO: Wert erfasst...");
    Serial.printf("Akkuspannung: %f V\n", battery_value);

    mqtt_client.publish(TOPIC_BATTERY, String(battery_value).c_str());

    /*
    ** Tag bzw. Nacht ermitteln und Tag- bzw. Nachtmodus aktivieren
    */
    daytime = analogRead(4) < 255;  
  }
  else
  {
    Serial.println("INFO: Wert erfasst...");
    Serial.printf("Niederschlag: %f mm\n", RAIN_LEVEL);

    /*
    ** Konstanten Wert (0,272727273 mm/Tick) übertragen
    */
    mqtt_client.publish(TOPIC_RAIN, String(RAIN_LEVEL).c_str());
  }
  
    delay(2000);

    Serial.println("INFO: Deepsleep beginnt in kürze...");
    Serial.flush();

    /*
    ** Verbindungen zum AP und MQTT Broker trennen, um IO- und Verbindungsfehler zu vermeiden
    */
    mqtt_client.disconnect();
    WiFi.disconnect();
    
    delay(100);

    esp_deep_sleep_start();
}

void loop() {}