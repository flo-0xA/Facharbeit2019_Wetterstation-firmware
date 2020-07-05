#include <Arduino.h>
#include "initialization.h"

#define SENSOR_WIND 18

#define SEA_LEVEL_PREPRESSURE 1013.25
#define RAIN_LEVEL 0.272727273

#define TOPIC_TEMPERATURE ""
#define TOPIC_HUMIDITY ""
#define TOPIC_PRESSURE ""
#define TOPIC_WIND_SPEED ""
#define TOPIC_WIND_DIRECTION ""
#define TOPIC_RAIN ""


RTC_DATA_ATTR uint8_t boot_count = 0;


const char* direction_table[] = {"N","NNE","NE","ENE","E","ESE","SE","SSE","S","SSW","SW","WSW","W","WNW","NW","NNW"};
int direction;
float speed;
 
bool status;
float temperature;
float humidity;
float pressure;


void setup() {
  deepsleep_init();


}

void loop() {}