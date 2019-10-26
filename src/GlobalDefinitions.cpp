/*
 *  Hier werden alle Konstanten global definiert.
 */
#include "GlobalDefinitions.hpp"

/* Dauer (in Sekunden) des Deepsleeps */
short TIME_TO_SLEEP = 60;

/* Konfiguratione der WLAN-Verbindung */
char* WLAN_SSID = "IoTify";
char* WLAN_KEY = "wBox2019";

/* Konfiguration für OTA-Update */
char* OTA_HOST = "";
int OTA_PORT = 80;
char* OTA_FIRMWARE = "";

/* Konfiguration für MQTT */
char* MQTT_HOST = "192.168.178.46";
int MQTT_PORT = 1883;

char* TOPIC_TEMPERATURE = "/smart/weather/temp";
char* TOPIC_HUMIDITY = "/smart/weather/hum";
char* TOPIC_PRESSURE = "/smart/weather/pres";
char* TOPIC_WIND_SPEED = "/smart/weather/wind_speed";
char* TOPIC_WIND_DIRECTION = "/smart/weather/wind_direction";
char* TOPIC_RAIN = "/smart/weather/rain";
char* TOPIC_BATTERY = "/smart/weather/bat";
char* TOPIC_UVA = "/smart/weather/uvA";
char* TOPIC_UVB = "/smart/weather/uvB";
char* TOPIC_UVI = "/smart/weather/uvI";