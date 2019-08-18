/*
 *  Hier werden alle Konstanten global definiert.
 */
#include "GlobalDefinitions.hpp"

/* Dauer (in Sekunden) des Deepsleeps */
short TIME_TO_SLEEP = 60;

/* Konfiguratione der WLAN-Verbindung */
char* WLAN_SSID = "IoTify";
char* WLAN_KEY = "wBox2019";

char* OTA_HOST = "";
char* OTA_FIRMWARE = "";

/* Konfiguration für MQTT */
char* MQTT_BROKER = "192.168.178.46";

char* TOPIC_TEMPERATURE = "/wetterstation/temperature";
char* TOPIC_HUMIDITY = "/wetterstation/humidity";
char* TOPIC_PRESSURE = "/wetterstation/pressure";
char* TOPIC_WIND_SPEED = "/wetterstation/wind_speed";
char* TOPIC_WIND_DIRECTION = "/wetterstation/wind_direction";
char* TOPIC_RAIN = "/wetterstation/rain";
char* TOPIC_BATTERY = "/wetterstation/battery";