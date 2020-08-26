#ifndef __SETUP_H__
#define __SETUP_H__

/*
 * OTA-Modus definieren
 *  0 = Update wird von einem Server geladen
 *  1 = Update über Webportal
 */
#define OTA_MODE 0

#define TOPIC_TEMPERATURE "/weatherstation/temperature"
#define TOPIC_HUMIDITY "/weatherstation/humidity"
#define TOPIC_PRESSURE "/weatherstation/pressure"
#define TOPIC_UVA "/weatherstation/uv/a"
#define TOPIC_UVB "/weatherstation/uv/b"
#define TOPIC_UV_INDEX "/weatherstation/uv/index"
#define TOPIC_WIND_SPEED "/weatherstation/wind/speed"
#define TOPIC_RAIN "/weatherstation/rain"
#define TOPIC_BATTERY "/weatherstation/battery"
#define TOPIC_ERROR "/weatherstation/error"

#define HOSTNAME "wetter-station"

#define OTA_SERVER "ota-server"
#define OTA_PORT 12345
#define OTA_PASSWORD "123456789"

#define STATION "wlan-ap"
#define KEY "123456789"

#define MQTT_SERVER "mqtt-broker"
#define MQTT_PORT 12345
#define MQTT_USER ""
#define MQTT_PASSWORD ""

#endif